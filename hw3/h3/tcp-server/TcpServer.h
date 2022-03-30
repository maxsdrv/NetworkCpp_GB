#ifndef CPP_NETWORK_LESSONS_TCPSERVER_H
#define CPP_NETWORK_LESSONS_TCPSERVER_H

#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <string>
#include <unistd.h>
#include <cerrno>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <cstring>
#include <csignal>

#include <socket_wrapper/socket_headers.h>
#include <socket_wrapper/socket_wrapper.h>
#include <socket_wrapper/socket_class.h>


class TcpServer
{
public:
    explicit TcpServer(const socket_wrapper::Socket& socket, const int port);
    ~TcpServer();
    void start();
    void init();
    bool check_gai();
    int check_socket();
    void bind_socket();

private:
    socket_wrapper::SocketWrapper m_sock_wrap;
    socket_wrapper::Socket m_socket;
    struct addrinfo hints, *info;
    sockaddr_in m_addr;
    int gai_result;
    char hostname[1024];
    int m_port;
};


#endif //CPP_NETWORK_LESSONS_TCPSERVER_H
