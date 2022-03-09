#include "TcpServer.h"

#include <algorithm>

// Trim from end (in place).
static inline void rtrim(std::string &s)
{
    s.erase(std::find_if(s.rbegin(), s.rend(),
                         std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
}

TcpServer::TcpServer(const socket_wrapper::Socket& socket, const int port) : m_socket(socket), hints({0}),
                                                                           info(nullptr), m_addr({0}), gai_result(0),
                                                                           hostname(), m_port(port)
{
}

void TcpServer::start()
{
    char buffer[256];

    // socket address used to store client address
    struct sockaddr_in client_address = {0};
    socklen_t client_address_len = sizeof(sockaddr_in);

    ssize_t recv_len = 0;

    std::cout << "Running echo server...\n" << std::endl;

    while (true)
    {
        // Get incoming connection through accept()
        int newsocket;
        if ((newsocket = accept(m_socket, reinterpret_cast<sockaddr *>(&client_address), &client_address_len)) < 0)
        {

        }


        if (!newsocket)
        {
            throw std::runtime_error("ERROR on accept");
        }

        // Read content into buffer from an incoming client.
        recv_len = recvfrom(m_socket, buffer, sizeof(buffer), 0,
                            reinterpret_cast<sockaddr *>(&client_address),
                            &client_address_len);


        if (recv_len > 0)
        {

            buffer[recv_len] = '\0';
            std::cout
                    << "Client with address "
                    << inet_ntoa(client_address.sin_addr)
                    << ":" << client_address.sin_port
                    << " hostname: "
                    << info->ai_canonname //it's a hostname
                    << " sent datagram "
                    << "[length = "
                    << recv_len
                    << "]:\n'''\n"
                    << buffer
                    << "\n'''";



            // Send same content back to the client ("echo").
            sendto(m_socket, buffer, recv_len, 0, reinterpret_cast<const sockaddr *>(&client_address),
                   client_address_len);
        }
        else
        {
            std::cerr << m_sock_wrap.get_last_error_string() << std::endl;
            return;
        }

        std::cout << std::endl;
    }
}

void TcpServer::init()
{
    hostname[1023] = '\0';
    gethostname(hostname, 1023);

    std::memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_CANONNAME;

    if (!check_gai())
    {
        std::cerr << stderr << gai_strerror(gai_result) << '\n';
        exit(1);
    }

    if (check_socket() < 0)
    {
        throw std::runtime_error("ERROR opening socket");
    }

    m_addr =
            {
                    .sin_family = AF_INET,
                    .sin_port = htons(m_port),
                    .sin_addr = {.s_addr = INADDR_ANY}
            };
}

bool TcpServer::check_gai()
{
    return ((gai_result = getaddrinfo(hostname, "http", &hints, &info)) == 0); //using modern function getaddrinfo
}
int TcpServer::check_socket()
{
    return m_socket;
}

void TcpServer::bind_socket()
{
    //bind a socket to a listening port
    if (bind(m_socket, reinterpret_cast<const sockaddr*>(&m_addr), sizeof(m_addr)) < 0)
    {
        std::cerr << m_sock_wrap.get_last_error_string() << std::endl;

        throw std::runtime_error("Bind error");
    }
    listen(m_socket, 5); //prepare a socket for waiting
}
