//
// Created by maxim on 13.02.2022.
//

#include "SFileLogger.h"

void SFileLogger::OpenLogFile(const std::string &file) {
    log_out.open(file, std::ios_base::out);
}

void SFileLogger::CloseLogFile() {

}

void SFileLogger::WriteToLog() {

}
