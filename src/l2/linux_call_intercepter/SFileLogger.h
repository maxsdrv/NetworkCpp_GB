//
// Created by maxim on 13.02.2022.
//

#ifndef CPP_NETWORK_LESSONS_SFILELOGGER_H
#define CPP_NETWORK_LESSONS_SFILELOGGER_H

#include <string>
#include <fstream>
#include <iostream>


class SFileLogger {
public:
    static SFileLogger& getInstance()
    {
        static SFileLogger theInstance;
        return theInstance;
    }
    void OpenLogFile(const std::string& file);

    void CloseLogFile();

    void WriteToLog();
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
