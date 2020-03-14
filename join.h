#pragma once

////////////////////////////////////////////////////////////////////////////////
// Combine a sequence of stringifyables, placing a delimiter between each
// items string representation.
////////////////////////////////////////////////////////////////////////////////

#include <functional>
#include <iterator>
#include <numeric>
#include <sstream>
#include <string>

const struct Join {
    template <class C>
    std::string operator()(std::string delim, const C& c) const {
        std::ostringstream os;
        auto it = std::begin(c);
        if (it != std::end(c)) {
            os << *it;
            accumulate(++it, std::end(c), std::ref(os), 
                       [=](std::ostringstream& acc, auto& x) {
                           acc << delim << x;
                           return std::ref(acc);
                       });
        }
        return os.str();
    }
} join;
