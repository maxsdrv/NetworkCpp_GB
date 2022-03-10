#include <iostream>

#include "TcpClient.h"



int main(int argc, char const *argv[])
{
    socket_wrapper::Socket socket1(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    const std::string host_name = {argv[1]};
    const int port {std::stoi(argv[2])};

    TcpClient client(socket1, host_name, port);
    client.init();
    client.start_msg();

    return EXIT_SUCCESS;
}

