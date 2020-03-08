#pragma once

////////////////////////////////////////////////////////////////////////////////
// The iterator pointing to the first element of the given sequence that
// matches (per ==) the item sought or, if no such element is found, the
// iterator std::end(that sequence)
////////////////////////////////////////////////////////////////////////////////

#include <algorithm>
#include <iterator>

const struct Find {
    template <class C, class X>
    auto operator()(C& c, const X& x) const {
        return std::find(std::begin(c), std::end(c), x);
    }
} find;
