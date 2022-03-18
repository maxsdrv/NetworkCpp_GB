#ifndef CLIENT_H
#define CLIENT_H

#include "Server.h"

class Client
{
public:
    explicit Client(socket_wrapper::Socket &&sock);

    std::optional<fs::path> recv_file_path();
    int recv_file_size();

    bool send_file(const fs::path &file_path);
    bool process();

private:
    Server srv_;
    fs::path file_path_;
};


#endif //CLIENT_H
