#pragma once

////////////////////////////////////////////////////////////////////////////////
// The concatenation of two sequences
////////////////////////////////////////////////////////////////////////////////

#include <algorithm>
#include <count.h>
#include <iterator>
#include <type_traits>
#include <vector>

const struct Concat {
    template <class X, class Y>
    auto operator()(const X& x, const Y& y) const {
        using R = std::common_type_t<std::decay_t<decltype(*std::begin(x))>,
                                     std::decay_t<decltype(*std::begin(y))>>;
        std::vector<R> r;
        r.reserve(count(x) + count(y));
        std::copy(std::begin(x), std::end(x), std::back_inserter(r));
        std::copy(std::begin(y), std::end(y), std::back_inserter(r));
        return r;
    }
} concat;
