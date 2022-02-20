#ifndef CPP_NETWORK_LESSONS_SFILELOGGER_H
#define CPP_NETWORK_LESSONS_SFILELOGGER_H

#include <string>
#include <fstream>
#include <iostream>


class SFileLogger {
public:
    static SFileLogger& get_instance()
    {
        static SFileLogger the_instance;
        return the_instance;
    }
    void open_log_file(const std::string& file);

    void close_log_file();

    void write_to_log(const char* info);
private:
    SFileLogger() = default;

    ~SFileLogger() = default;

    SFileLogger(const SFileLogger&) = delete;

    SFileLogger(SFileLogger&&) = delete;

    SFileLogger& operator=(const SFileLogger&) = delete;

    SFileLogger& operator=(SFileLogger&&) = delete;
private:
    std::ofstream log_out;
};


#endif //CPP_NETWORK_LESSONS_SFILELOGGER_H
