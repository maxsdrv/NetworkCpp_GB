#include "ReadableFormat.h"


std::ostream &operator<<(std::ostream &os, const ReadableFormat &rf)
{
    int i{};
    double mantissa = rf.size;
    for (; mantissa >= 1024.; mantissa /= 1024., ++i) { }
    mantissa = std::ceil(mantissa * 10.) / 10.;
    os << mantissa << "BKMGTPE"[i];
    return i == 0 ? os : os << "B (" << rf.size << ')';
}
