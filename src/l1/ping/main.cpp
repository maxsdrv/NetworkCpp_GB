#include <algorithm>
#include <cassert>
#include <chrono>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <string>
#include <thread>
#include <vector>

#ifdef _WIN32
#   include <process.h>
#   ifndef getpid
#       define getpid _getpid
#   endif
#else
#   include <unistd.h>
#endif

#include <socket_wrapper/socket_headers.h>
#include <socket_wrapper/socket_wrapper.h>
#include <socket_wrapper/socket_class.h>

using namespace std::chrono_literals;

// Define the Packet Constants
// ping packet size
const size_t ping_packet_size = 64;
const auto ping_sleep_rate = 1000000us;

// Gives the timeout delay for receiving packets.
const auto recv_timeout = 1s;

// Echo Request.
const int ICMP_ECHO = 8;

// Echo Response.
const int ICMP_ECHO_REPLY = 0;


#pragma pack(push, 1)
// Windows doesn't have this structure.
struct icmphdr
{
    uint8_t type;        /* message type */
    uint8_t code;        /* type sub-code */
    uint16_t checksum;
    union
    {
        struct
        {
            uint16_t    id;
            uint16_t    sequence;
        } echo;            /* echo datagram */
        uint32_t    gateway;    /* gateway address */
        struct
        {
            uint16_t    __unused;
            uint16_t    mtu;
        } frag;            /* path mtu discovery */
    } un;
};


//
// IPv4 Header (without any IP options)
//
typedef struct ip_hdr
{
    unsigned char  ip_verlen;        // 4-bit IPv4 version
                                     // 4-bit header length (in 32-bit words)
    unsigned char  ip_tos;           // IP type of service
    unsigned short ip_totallength;   // Total length
    unsigned short ip_id;            // Unique identifier 
    unsigned short ip_offset;        // Fragment offset field
    unsigned char  ip_ttl;           // Time to live
    unsigned char  ip_protocol;      // Protocol(TCP,UDP etc)
    unsigned short ip_checksum;      // IP checksum
    unsigned int   ip_srcaddr;       // Source address
    unsigned int   ip_destaddr;      // Source address
} IPV4_HDR, *PIPV4_HDR;
#pragma pack(pop)


// Ping packet structure.
class PingPacket
{
public:
    typedef std::vector<uint8_t> BufferType;

public:
    // Zero packet constructor. Need for recv.
    PingPacket(size_t packet_size = ping_packet_size) :
        packet_size_{packet_size}
    {
        data_buffer_.resize(packet_size_);
    }

    PingPacket(uint16_t packet_id, uint16_t packet_sequence_number, size_t packet_size = ping_packet_size) :
        packet_size_{packet_size}
    {
        create_new_packet(packet_id, packet_sequence_number);
    }

    PingPacket(BufferType &&packet_buffer) :
        packet_size_{packet_buffer.size()},
        data_buffer_{packet_buffer}
    {
        assert(packet_size_ >= sizeof(icmphdr));
    }

    PingPacket(const BufferType::iterator &start, const BufferType::iterator &end) :
        packet_size_{static_cast<size_t>(std::distance(end, start))},
        data_buffer_{start, end}
    {
        assert(packet_size_ >= sizeof(icmphdr));
    }

    const icmphdr &header() const
    {
        return *get_header_from_buffer();
    }

    size_t size()
    {
        return data_buffer_.size();
    }

    uint16_t checksum() const
    {
        const uint16_t *buf = reinterpret_cast<const uint16_t*>(data_buffer_.data());
        long int len = data_buffer_.size();
        uint32_t sum = 0;

        for (sum = 0; len > 1; len -= 2) sum += *buf++;
        if (1 == len) sum += *reinterpret_cast<const uint8_t*>(buf);
        sum = (sum >> 16) + (sum & 0xffff);
        sum += (sum >> 16);

        uint16_t result = sum;

        return ~result;
    }

public:
    operator BufferType() const
    {
        return data_buffer_;
    }

    operator const BufferType::value_type*() const
    {
        return data_buffer_.data();
    }

    operator BufferType::value_type*()
    {
        return data_buffer_.data();
    }

    operator bool() const
    {
        auto real_header = const_cast<icmphdr*>(get_header_from_buffer());
        auto old_checksum = real_header->checksum;

        // Checksum field in the packet must be equal to 0 before checksum calculation.
        real_header->checksum = 0;
        auto result = checksum();
        real_header->checksum = old_checksum;

        return old_checksum == result;
    }

private:
    icmphdr *get_header_from_buffer() const
    {
        return reinterpret_cast<icmphdr*>(const_cast<uint8_t*>(data_buffer_.data()));
    }

    void create_new_packet(uint16_t packet_id, uint16_t packet_sequence_number)
    {
        static_assert(sizeof(BufferType::value_type) == 1);
        assert(packet_size_ > sizeof(icmphdr));

        data_buffer_.resize(packet_size_);

        auto header = get_header_from_buffer();

        header->type = ICMP_ECHO;
        header->code = 0;
        header->checksum = 0;
        header->un.echo.id = htons(packet_id);
        header->un.echo.sequence = htons(packet_sequence_number);

        std::generate(std::next(data_buffer_.begin(), sizeof(icmphdr)), data_buffer_.end(),
            [i = 'a']() mutable
            {
                return i <= 'z' ? i++ : i = 'a';
            }
        );

        get_header_from_buffer()->checksum = checksum();
    }

private:
    const size_t packet_size_;
    BufferType data_buffer_;
};


class PingPacketFactory
{
public:
    const uint16_t max_id = 2 ^ (8 * sizeof(uint16_t)) - 1;

public:
    PingPacketFactory() : pid_(getpid()), sequence_number_{0} {}

public:
    PingPacket create_request()
    {
        return std::move(PingPacket(pid_, sequence_number_++));
    }

    PingPacket create_response()
    {
        return PingPacket();
        return std::move(PingPacket());
    }

private:
    int pid_;
    uint16_t sequence_number_;
};


void send_ping(const socket_wrapper::Socket &sock, const std::string &hostname, const struct sockaddr_in &host_address)
{
    int ttl_val = 255, msg_count=0, i, flag=1, hdr, msg_received_count=0;

    struct timeval tv =
    {
        .tv_sec = std::chrono::seconds(recv_timeout).count(),
        .tv_usec = (std::chrono::milliseconds(recv_timeout) - std::chrono::milliseconds(std::chrono::seconds(recv_timeout))).count()
    };
    struct sockaddr_in r_addr;

    PingPacketFactory ping_factory;

    // Possible to disable IP header:
    // setsockopt(sock, 0, IP_HDRINCL, &flag, sizeof(flag));
    // Set socket options at ip to TTL and value to 64,.
    if (setsockopt(sock, SOL_IP, IP_TTL, &ttl_val, sizeof(ttl_val)) != 0)
    {
        throw std::runtime_error("TTL setting failed!");
    }

    // Setting timeout of recv setting.
    if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char*>(&tv), sizeof(tv)) != 0)
    {
        throw std::runtime_error("Recv timeout setting failed!");
    }

    // send ICMP packet in an infinite loop
    while(true)
    {
        auto &&request = ping_factory.create_request();
        auto request_echo_header = request.header().un.echo;

        std::cout
            << "Sending packet "
            << ntohs(request_echo_header.sequence)
            << " to \""
            << hostname
            << "\" "
            << "request with id = "
            << ntohs(request_echo_header.id)
            << std::endl;

        if (sendto(sock, request, request.size(), 0, reinterpret_cast<const struct sockaddr*>(&host_address),
            sizeof(host_address)) < request.size())
        {
            std::cerr << "Packet sending failed: \"" << "\"" << std::endl;
            continue;
        }

        //auto &&response = ping_factory.create_response();

        // Receive packet
        socklen_t addr_len = sizeof(sockaddr);
        r_addr.sin_family = AF_INET;
        r_addr.sin_addr = host_address.sin_addr;

        std::vector<uint8_t> buffer;

        buffer.resize(ping_packet_size + 20);
        auto start_time = std::chrono::steady_clock::now();

        if (recvfrom(sock, buffer.data(), buffer.size(), 0, reinterpret_cast<struct sockaddr*>(&r_addr), &addr_len) < buffer.size())
        {
            std::cerr << "Packet receiving failed: \"" << "\"" << std::endl;
            continue;
        }

        auto end_time = std::chrono::steady_clock::now();

        // Skip IP header.
        auto response = PingPacket(buffer.begin() + (reinterpret_cast<const struct ip_hdr*>(buffer.data())->ip_verlen & 0x0f) * sizeof(uint32_t), buffer.end());
        if ((ICMP_ECHO_REPLY == response.header().type) and (0 == response.header().code))
        {
            auto response_echo_header = response.header().un.echo;
            std::cout
                << "Receiving packet "
                << ntohs(response_echo_header.sequence)
                << " from \""
                << hostname
                << "\" "
                << "response with id = "
                << ntohs(response_echo_header.id)
                << ", time = "
                << std::round(std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(end_time - start_time).count() * 10) / 10
                << "ms"
                << std::endl;
        }

        std::this_thread::sleep_for(ping_sleep_rate);
    }
}


int main(int argc, const char *argv[])
{

    if (argc != 2)
    {
        std::cout << "Usage: " << argv[0] << " <node>" << std::endl;
        return EXIT_FAILURE;
    }

    socket_wrapper::SocketWrapper sock_wrap;
    const std::string host_name = { argv[1] };
    const struct hostent *remote_host { gethostbyname(host_name.c_str()) };

    if (nullptr == remote_host)
    {
        if (sock_wrap.get_last_error_code())
        {
            std::cerr << sock_wrap.get_last_error_string() << std::endl;
        }

        return EXIT_FAILURE;
    }

    struct sockaddr_in addr =
    {
        .sin_family = static_cast<short unsigned>(remote_host->h_addrtype),
        // Automatic port number.
        .sin_port = htons(0),
        .sin_addr = { .s_addr = *reinterpret_cast<const in_addr_t*>(remote_host->h_addr) }
    };

    std::cout << "Pinging \"" << remote_host->h_name << "\" [" << inet_ntoa(addr.sin_addr) << "]" << std::endl;

    socket_wrapper::Socket sock = {AF_INET, SOCK_RAW, IPPROTO_ICMP};

    if (!sock)
    {
        std::cerr << sock_wrap.get_last_error_string() << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << "Start to sending packets..." << std::endl;
    // Send pings continuously.
    send_ping(sock, host_name, addr);

    return EXIT_SUCCESS;
}

