#ifndef SERVER_H
#define SERVER_H

#include <algorithm>
#include <cassert>
#include <filesystem>
#include <fstream>
#include <future>
#include <iostream>
#include <list>
#include <optional>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#include <cerrno>

#include <socket_wrapper/socket_headers.h>
#include <socket_wrapper/socket_wrapper.h>
#include <socket_wrapper/socket_class.h>

#if !defined(MAX_PATH)
#   define MAX_PATH (256)
#endif

namespace fs = std::filesystem;
using namespace std::literals;

const uint16_t buffer_size = 4096;
const uint16_t clients_count = 10;

#if defined(_WIN32)
const wchar_t separ = fs::path::preferred_separator;
#else
const wchar_t separ = *reinterpret_cast<const wchar_t*>(&fs::path::preferred_separator);
#endif

class Server
{
public:
    explicit Server(socket_wrapper::Socket &&client_sock);
    Server(const Server&) = delete;
    Server() = delete;

public:
    const socket_wrapper::Socket &ts_socket()const;

public:
    bool send_buffer(const std::vector<char> &buffer);
    bool send_file(fs::path const& file_path);
    bool send_file2(fs::path const& file_path, int size);
    std::string get_request();
    int get_request2();
private:
    static bool need_to_repeat();

private:
    socket_wrapper::Socket client_sock_;
};


#endif //SERVER_H













