#include <iostream>
#include <tag.h>

std::ostream& operator<<(std::ostream& os, Tag t) {
    constexpr char c[] = "ATVKRC";
    static_assert(END_TAGS == sizeof c - 1, "END_TAGS != # Tags in op<<");
    return os << c[std::size_t(t)];
}
