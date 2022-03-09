#include <iostream>
#include "TcpServer.h"



int main(int argc, char const *argv[])
{
    std::cout << "Homework started\n";

    if (argc != 2)
    {
        std::cout << "Usage: " << argv[0] << " <port>" << std::endl;
        return EXIT_FAILURE;
    }

    
    socket_wrapper::SocketWrapper sock_wrap;
    const int port { std::stoi(argv[1]) };

    socket_wrapper::Socket sock = {AF_INET, SOCK_STREAM, IPPROTO_TCP}; //create tcp socket

    std::cout << "Starting echo server on the port " << port << "...\n";

    TcpServer tcp_srv(sock, port);
    tcp_srv.init();
    tcp_srv.bind_socket();
    tcp_srv.start();

    return EXIT_SUCCESS;
}

