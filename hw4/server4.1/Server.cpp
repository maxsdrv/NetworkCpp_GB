#include "Server.h"
#include "ReadableFormat.h"

#include <sstream>

Server::Server(socket_wrapper::Socket &&client_sock) : client_sock_(std::move(client_sock))
{

}

const socket_wrapper::Socket &Server::ts_socket() const
{
    return client_sock_;
}

bool Server::send_buffer(const std::vector<char> &buffer)
{
    size_t transmit_bytes_count = 0;
    size_t count_bytes = 0;
    const auto size = buffer.size();

    while (transmit_bytes_count != size)
    {
        auto result = send(client_sock_, &(buffer.data()[0]) + transmit_bytes_count, size - transmit_bytes_count, 0);

        if (-1 == result)
        {
            if (need_to_repeat()) continue;
            return false;
        }

        transmit_bytes_count += result;
    }

    return true;
}

bool Server::send_file2(const fs::path &file_path, int _size)
{
    std::vector<char> buffer(buffer_size);

    std::ifstream file_stream(file_path, std::ifstream::binary);

    std::cout << "size = " << ReadableFormat{fs::file_size(file_path)} << std::endl;

    if (fs::file_size(file_path) < _size) return false;

    if (!file_stream)
    {
        std::cerr
            << "Size of package which you"
               " want transfer greater then"
               " file size exists" << std::endl;
    }

    std::cout << "Sending file " << file_path << "..." << std::endl;

    // Transform file in parts
    const uint16_t package = 4096;
    if (_size < package)
    {
        std::cerr
            << "SIZE::FILE::PACKAGE::ERROR\n"
            << " Size package which you would "
            << "transfer must be greater than 4096 bytes\n" << std::endl;
        assert(_size >= package);
    }

    auto iter = _size / package;
    auto rest_file = fs::file_size(file_path) % _size;

    for (size_t i = 0; i < iter; ++i)
    {
        file_stream.read(buffer.data(), buffer.size());
        if (!send_buffer(buffer)) return false;
    }

    std::cout << "Rest file for download = " << ReadableFormat{(fs::file_size(file_path) + rest_file) - _size} << std::endl;

    fs::resize_file(file_path, (fs::file_size(file_path) + rest_file) - _size);
    if (fs::file_size(file_path) < package)
    {
        file_stream.readsome(buffer.data(), buffer.size());
        if (!send_buffer(buffer)) return false;
        std::cout << "The entire file transfered successfull\n";
    }

    return true;
}

bool Server::send_file(const fs::path &file_path)
{
    std::vector<char> buffer(buffer_size);
    std::ifstream file_stream(file_path, std::ifstream::binary);

    std::cout << "size = " << ReadableFormat{fs::file_size(file_path)} << std::endl;

    if (!file_stream) return false;

    std::cout << "Sending file " << file_path << "..." << std::endl;
    while (file_stream)
    {
        file_stream.read(&buffer[0], buffer.size());
        if (!send_buffer(buffer)) return false;
    }

    return true;
}

int Server::get_request2()
{
    std::array<char, MAX_PATH + 1> buffer{};
    size_t recv_bytes = 0;
    std::string temp_str {};

    const auto size = buffer.size() - 1;

    std::cout << "Reading user request file size..." << std::endl;
    while (true)
    {
        auto result = recv(client_sock_, &buffer[recv_bytes], size - recv_bytes, 0);

        if (!result) break;

        if (-1 == result)
        {
            if (need_to_repeat()) continue;
            throw std::logic_error("Socket reading error");
        }

        auto fragment_begin = buffer.begin() + recv_bytes;

        auto file_arg = std::find_if(fragment_begin, fragment_begin + result,
                                      [](char sym) { return '\n' == sym || '\r' == sym ; });



        if (file_arg != buffer.end())
        {
            *file_arg = '\0';

            recv_bytes += std::distance(fragment_begin, file_arg);
            break;
        }
        recv_bytes += result;
        if (size == recv_bytes) break;
    }

    buffer[recv_bytes] = '\0';

    auto str = std::string(buffer.begin(), buffer.begin() + recv_bytes);

    std::string::size_type pos = str.find(' ');
    temp_str = str.substr(0, pos);
    str = str.substr(pos + 1, std::string::npos);

    std::cout << "Request = \"" << str << "\"" << std::endl;

    return std::stoi(str, nullptr);
}

std::string Server::get_request()
{
    std::array<char, MAX_PATH + 1> buffer;
    size_t recv_bytes = 0;
    const auto size = buffer.size() - 1;

    std::cout << "Reading user request file path..." << std::endl;
    while (true)
    {
        auto result = recv(client_sock_, &buffer[recv_bytes], size - recv_bytes, 0);

        if (!result) break;

        if (-1 == result)
        {
            if (need_to_repeat()) continue;
            throw std::logic_error("Socket reading error");
        }

        auto fragment_begin = buffer.begin() + recv_bytes;
        auto ret_iter = std::find_if(fragment_begin, fragment_begin + result,
                                     [](char sym) { return '\n' == sym || '\r' == sym ;  });
        if (ret_iter != buffer.end())
        {
            *ret_iter = '\0';
            recv_bytes += std::distance(fragment_begin, ret_iter);
            break;
        }
        recv_bytes += result;
        if (size == recv_bytes) break;
    }

    buffer[recv_bytes] = '\0';

    auto result = std::string(buffer.begin(), buffer.begin() + recv_bytes);
    std::cout << "Request = \"" << result << "\"" << std::endl;

    return result;
}

bool Server::need_to_repeat()
{
    switch (errno)
    {
    case EINTR:
    case EAGAIN:
        // case EWOULDBLOCK: // EWOULDBLOCK == EINTR.
        return true;
    }

    return false;
}











