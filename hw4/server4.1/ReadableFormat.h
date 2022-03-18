#ifndef READABLEFORMAT_H
#define READABLEFORMAT_H

#include <cmath>
#include <filesystem>
#include <iostream>

class ReadableFormat
{
public:
    std::uintmax_t size{};

private:
     friend std::ostream& operator<<(std::ostream& os, const ReadableFormat& rf);
};


#endif //READABLEFORMAT_H
