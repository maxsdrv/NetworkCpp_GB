#include "Client.h"
#include "CommonFunction.h"

#if !defined(_WIN32)
extern "C"
{
#   include <signal.h>
}
#else
#   include <cwctype>
#endif



int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        std::cout << "Usage: " << argv[0] << " <port>" << std::endl;
        return EXIT_FAILURE;
    }

#if !defined(_WIN32)
    signal(SIGPIPE, SIG_IGN);
#endif

    socket_wrapper::SocketWrapper sock_wrap;

    try
    {
        auto servinfo = get_serv_info(argv[1]);
        if (!servinfo)
        {
            std::cerr << "Can't get servinfo!" << std::endl;
            exit(EXIT_FAILURE);
        }

        socket_wrapper::Socket server_sock = {servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol};

        if (!server_sock)
        {
            throw std::logic_error("Socket creation error");
        }

        set_reuse_addr(server_sock);

        if (bind(server_sock, servinfo->ai_addr, servinfo->ai_addrlen) < 0)
        {
            throw std::logic_error("Bind error");
        }

        std::list<std::future<bool>> pending_tasks;

        std::cout
            << "Listening on port " << argv[1] << "...\n"
            << "Server path: " << fs::current_path()
            << std::endl;

        if (listen(server_sock, clients_count) < 0)
        {
            throw std::logic_error("Listen error");
        }

        std::cout << "Listen was run..." << std::endl;

        while (true)
        {
            auto client_sock = accept_client(server_sock);

            if (!client_sock)
            {
                throw std::logic_error("Client socket error");
            }

            pending_tasks.push_back(std::async(std::launch::async, [&](socket_wrapper::Socket &&sock)
            {
                Client client(std::move(sock));
                std::cout << "Client tid = " << std::this_thread::get_id() << std::endl;
                auto result = client.process();
                std::cout
                    << "Client with tid = " << std::this_thread::get_id()
                    << " exiting..."
                    << std::endl;

                return result;
            }, std::move(client_sock)));

            std::cout << "Cleaning tasks..." << std::endl;
            for (auto task = pending_tasks.begin(); task != pending_tasks.end();)
            {
                if (std::future_status::ready == task->wait_for(1ms))
                {
                    auto fu = task++;
                    std::cout
                        << "Request completed with a result = " << fu->get() << "...\n"
                        << "Removing from list." << std::endl;
                    pending_tasks.erase(fu);
                }
                else ++task;
            }
        }
    }
    catch (const std::logic_error &e)
    {
        std::cerr
            << e.what()
            << " [" << sock_wrap.get_last_error_string() << "]!"
            << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

