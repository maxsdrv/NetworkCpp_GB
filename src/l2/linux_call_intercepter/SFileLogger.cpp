//
// Created by maxim on 13.02.2022.
//

#include "SFileLogger.h"

void SFileLogger::open_log_file(const std::string &file)
{
    log_out.open(file, std::ios_base::out);
}

void SFileLogger::close_log_file()
{
    if (log_out.is_open())
    {
        log_out.close();
    }
}

void SFileLogger::write_to_log(const char* info)
{
    if (log_out.is_open())
    {
        log_out << "New message: " << info << std::endl;
    }
}
