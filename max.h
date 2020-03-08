#pragma once

////////////////////////////////////////////////////////////////////////////////
// Maximum of two items or of a sequence
////////////////////////////////////////////////////////////////////////////////

#include <algorithm>
#include <cassert>
#include <count.h>

const struct Max {
    template <class C>
    auto operator()(const C& x) const {
        assert(count(x));
        return *std::max_element(std::begin(x), std::end(x));
    }

    template <class X>
    auto operator()(const X& x, const X& y) const {
        return std::max(x, y);
    }
} max;
