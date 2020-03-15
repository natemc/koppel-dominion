#include <event.h>
#include <card.h>
#include <iostream>

std::ostream& operator<<(std::ostream& os, const Event& e) {
    os << e.description << " event";
    if (e.card) os << ' ' << *e.card;
    return os;
}
