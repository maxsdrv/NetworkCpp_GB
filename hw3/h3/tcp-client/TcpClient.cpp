#include "TcpClient.h"


TcpClient::TcpClient(const socket_wrapper::Socket &socket, const std::string& host_name, const int port) :
                                                                             m_socket(socket),
                                                                             buffer(), serv_addr({0}),
                                                                             cli({0}), m_port(port),
                                                                             m_host_name(host_name)
{

}

TcpClient::~TcpClient()
{
    std::cout << "~TcpClient()\n";
}

void TcpClient::init()
{
    if (!m_socket)
    {
        std::cerr << m_wrapper.get_last_error_string() << std::endl;
        throw std::runtime_error("SOCKET::FAILED\n");
    }

    std::memset(&serv_addr, 0, sizeof serv_addr);

    const struct hostent *remote_host {gethostbyname(m_host_name.c_str())};

    serv_addr =
        {
            .sin_family = AF_INET,
            .sin_port = htons(m_port),
//            .sin_addr = {.s_addr = INADDR_ANY}
        };
    serv_addr.sin_addr.s_addr = *reinterpret_cast<const in_addr_t*>(remote_host->h_addr);

    //connect the client socket to server socket
    if (connect(m_socket, reinterpret_cast<const sockaddr* const>(&serv_addr), sizeof serv_addr) != 0)
    {
        throw std::runtime_error("ERROR::CONNECTION::SERVER\n");
    }
}

void TcpClient::start_msg()
{
    std::string msg;
    buffer.resize(MAX_RECV_BUFFER_SIZE);
    std::cout << "CONNECTED::-> ' " << m_host_name << "\'..." << std::endl;

    while (true)
    {
        std::cout << "> " << std::flush;
        if (!std::getline(std::cin, msg))
        {
            std::cout << "ERROR::SEND::MESSAGE\n";
            break;
        }

        std::cout << "Sending message to server [ " << msg << " ] ... \n";

        if (!send_request(m_socket, msg))
        {
            std::cerr << m_wrapper.get_last_error_string() << std::endl;
            break;
        }

        std::cout << "Request was sent, reading responce..." << std::endl;

        auto recv_bytes = recv(m_socket, buffer.data(), buffer.size() - 1, 0);

        if (recv_bytes > 0)
        {
            buffer[recv_bytes] = '\0';
            std::cout << "-------------\n" << std::string(buffer.begin(), std::next(buffer.begin(), recv_bytes)) << std::endl;

            if (msg == "exit")
            {
                std::cout << "Client exit ... \n";
                break;
            }

            continue;
        }
        else if (-1 == recv_bytes)
        {
            if (EINTR == errno) continue;
            if (0 == errno) break;
        }

    }

}
bool TcpClient::send_request(socket_wrapper::Socket &sock, const std::string& msg)
{
    ssize_t bytes_count = 0;
    size_t msg_pos = 0;
    auto const msg_buffer = &(msg.c_str()[0]);
    auto const msg_length = msg.length();

    while (true)
    {
        if ((bytes_count = send(m_socket, msg_buffer + msg_pos, msg_length - msg_pos, 0)) < 0)
        {
            if (EINTR == errno) continue;
        }
        else
        {
            if (!bytes_count) return false;

            msg_pos += bytes_count;

            if (msg_pos >= msg_length)
            {
                break;
            }
        }
    }
    return true;
}





