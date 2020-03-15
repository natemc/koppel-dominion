#pragma once

////////////////////////////////////////////////////////////////////////////////
// Non-destructive unstable sort of a sequence per the given ordering
////////////////////////////////////////////////////////////////////////////////

#include <algorithm>
#include <functional>
#include <iterator>
#include <utility>

const struct Sort {
    template <class X, class C>
    X operator()(X x) const {
        std::sort(std::begin(x), std::end(x));
        return x;
    }

    template <class X, class C>
    X operator()(X x, C&& cmp) const {
        std::sort(std::begin(x), std::end(x), std::forward<C>(cmp));
        return x;
    }
} sort;
