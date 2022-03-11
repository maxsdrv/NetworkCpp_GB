#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <chrono>
#include <exception>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <string>
#include <cstring>
#include <vector>

#include <socket_wrapper/socket_headers.h>
#include <socket_wrapper/socket_wrapper.h>
#include <socket_wrapper/socket_class.h>

const auto MAX_RECV_BUFFER_SIZE = 256;

class TcpClient
{
public:
    TcpClient(const socket_wrapper::Socket& socket, const std::string& host_name, const int port);
    ~TcpClient();

    void init();
    void start_msg();
    bool send_request(socket_wrapper::Socket& sock, const std::string& msg);

private:
    socket_wrapper::Socket m_socket;
    socket_wrapper::SocketWrapper m_wrapper;
    std::vector<char> buffer;
    struct sockaddr_in serv_addr, cli;
    int m_port;
    std::string m_host_name;
};


#endif //TCPCLIENT_H
