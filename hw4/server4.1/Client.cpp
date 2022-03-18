#include "Client.h"


Client::Client(socket_wrapper::Socket &&sock) : srv_(std::move(sock))
{
    std::cout
        << "Client ["<< static_cast<int>(srv_.ts_socket()) << "]"
        << "was created..."
        << std::endl;
}

int Client::recv_file_size()
{
    int file_size = srv_.get_request2();

    return file_size;
}

std::optional<fs::path> Client::recv_file_path()
{
    auto request_data= srv_.get_request();

    if (request_data.empty()) return std::nullopt;

    auto cur_path = fs::current_path().wstring();
    auto file_path = fs::weakly_canonical(request_data).wstring();

    if (file_path.find(cur_path) == 0)
    {
        file_path = file_path.substr(cur_path.length());
    }

    return fs::weakly_canonical(cur_path + separ + file_path);
}

bool Client::send_file(const fs::path &file_path)
{
    if (!(fs::exists(file_path) && fs::is_regular_file(file_path))) return false;

    auto file_size = recv_file_size();

    return srv_.send_file2(file_path, file_size);
}

bool Client::process()
{
    auto file_to_send = recv_file_path();

    bool result = false;

    if (std::nullopt != file_to_send)
    {
        std::cout << "Trying to send " << *file_to_send << "..." << std::endl;
        if (send_file(*file_to_send))
        {
            std::cout << "File was sent." << std::endl;
        }
        else
        {
            std::cerr << "File sending error!" << std::endl;
        }
        result = true;
    }

    return result;
}




