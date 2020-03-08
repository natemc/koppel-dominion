#pragma once

////////////////////////////////////////////////////////////////////////////////
// Combine a sequence of stringifyables, placing a delimiter between each
// items string representation.
////////////////////////////////////////////////////////////////////////////////

#include <functional>
#include <iterator>
#include <over.h>
#include <sstream>
#include <string>

const struct Join {
    template <class C>
    std::string operator()(std::string delim, const C& c) const {
        std::ostringstream os;
        auto combine = [=](std::ostringstream& acc, auto& x) {
            acc << delim << x;
            return std::ref(acc);
        };
        over(combine, std::ref(os), c);
        return os.str();
    }
} join;
