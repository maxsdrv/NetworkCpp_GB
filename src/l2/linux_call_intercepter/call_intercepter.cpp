#define _GNU_SOURCE

//
// System calls interceptor for the networking spoiling...
//

extern "C"
{
#include <dlfcn.h>
#include <unistd.h>
}

#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <fstream>

#include "SFileLogger.h"


static void init (void) __attribute__ ((constructor));

typedef ssize_t (*write_t)(int fd, const void *buf, size_t count);
typedef int (*socket_t)(int domain, int type, int protocol);
typedef int (*close_t)(int fd);

static close_t old_close;
static socket_t old_socket;
static write_t old_write;

static int socket_fd = -1;


void init(void)
{
    SFileLogger::getInstance().open_log_file("test_log.txt"); // open logging

    srand(time(nullptr));
    printf("Interceptor library loaded. \n");

    old_close = reinterpret_cast<close_t>(dlsym(RTLD_NEXT, "close"));
    old_write = reinterpret_cast<write_t>(dlsym(RTLD_NEXT, "write"));
    old_socket = reinterpret_cast<socket_t>(dlsym(RTLD_NEXT, "socket"));
}


extern "C"
{

int close(int fd)
{
    if (fd == socket_fd)
    {
        printf("> close() on the socket was called!\n");
        socket_fd = -1;
    }

    SFileLogger::getInstance().close_log_file(); // close logging

    return old_close(fd);
}


ssize_t write(int fd, const void *buf, size_t count)
{
    auto char_buf = reinterpret_cast<const char*>(buf);

    if (char_buf && (count > 1) && (fd == socket_fd))
    {
        printf("> write() on the socket was called with a string!\n");
        printf("New buffer = [");

        std::string str {}; //store of char symbols

        for (size_t i = 0; i < count - 1; ++i)
        {
            int r = rand();
            char *c = const_cast<char *>(char_buf) + i;

            // ASCII symbol.
            if (1 == r % count) *c = r % (0x7f - 0x20) + 0x20;

            str += *c; //record each symbols to string

            putchar(*c);
        }

        SFileLogger::getInstance().write_to_log(str.c_str()); // write to logging

        printf("]\n");
    }

    return old_write(fd, buf, count);
}


int socket(int domain, int type, int protocol)
{
    int cur_socket_fd = old_socket(domain, type, protocol);

    if (-1 == socket_fd)
    {
        printf("> socket() was called, fd = %d!\n", cur_socket_fd);
        socket_fd = cur_socket_fd;
    }
    else
    {
        printf("> socket() was called, but socket was opened already...\n");
    }

    return cur_socket_fd;
}

} // extern "C"

