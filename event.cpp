#include <event.h>
#include <iostream>

std::ostream& operator<<(std::ostream& os, const Event& e) {
    return os << e.description << " event";
}
