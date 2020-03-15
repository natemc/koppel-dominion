#pragma once

////////////////////////////////////////////////////////////////////////////////
// Combine a sequence of stringifyables, placing a delimiter between each
// items string representation.
////////////////////////////////////////////////////////////////////////////////

#include <functional>
#include <initializer_list>
#include <iterator>
#include <numeric>
#include <sstream>
#include <string>
#include <utility>

const struct Join {
    template <class C>
    std::string operator()(std::string delim, const C& c) const {
        return (*this)(std::move(delim), std::begin(c), std::end(c));
    }

    template <class X>
    std::string operator()(std::string delim, std::initializer_list<X> x) const {
        return (*this)(std::move(delim), std::begin(x), std::end(x));
    }

    template <class I>
    std::string operator()(std::string delim, I first, I last) const {
        std::ostringstream os;
        if (first != last) {
            os << *first;
            std::accumulate(++first, last, std::ref(os), 
                            [=](std::ostringstream& acc, auto& x) {
                                acc << delim << x;
                                return std::ref(acc);
                            });
        }
        return os.str();
    }
} join;
