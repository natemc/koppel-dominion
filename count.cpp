#include <count.h>
#include <cstring>

std::size_t Count::operator()(const char* s) const {
    return std::strlen(s);
}
