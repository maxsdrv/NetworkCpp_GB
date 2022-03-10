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
    std::cout << "CONNECTED::-> " << m_host_name << "\"..." << std::endl;

    while (true)
    {
        std::cout << "> " << std::flush;
        if (!std::getline(std::cin, msg))
        {
            std::cout << "ERROR::SEND::MESSAGE\n";
            break;
        }

        std::cout << "Sending message to server\n";

        write(m_socket, &buffer, buffer.size());
        buffer.resize(MAX_RECV_BUFFER_SIZE);
        read(m_socket, &buffer, buffer.size());

        std::cout << "From Server\n";

        for (const auto& i : buffer)
        {
            std::cout << i;
        }

        if ((std::strncmp(msg.c_str(), "exit", 4)) == 0)
        {
            std::cout << "Client exit ... \n";
            break;
        }
    }

}





