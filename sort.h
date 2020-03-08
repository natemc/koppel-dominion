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
    X operator()(const X& x) const {
        X r(x);
        std::sort(std::begin(r), std::end(r));
        return r;
    }

    template <class X, class C>
    X operator()(const X& x, C&& cmp) const {
        X r(x);
        std::sort(std::begin(r), std::end(r), std::forward<C>(cmp));
        return r;
    }
} sort;
