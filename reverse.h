#pragma once

////////////////////////////////////////////////////////////////////////////////
// Non-destructive reverse of a sequence
////////////////////////////////////////////////////////////////////////////////

#include <algorithm>
#include <iterator>

const struct Reverse {
    template <class C>
    C operator()(C x) const {
        std::reverse(std::begin(x), std::end(x));
        return x;
    }
} reverse;
