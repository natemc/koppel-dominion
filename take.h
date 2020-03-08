#pragma once

////////////////////////////////////////////////////////////////////////////////
// (Up to) the first n elements of a sequence
////////////////////////////////////////////////////////////////////////////////

#include <algorithm>
#include <cstddef>
#include <count.h>
#include <iterator>
#include <type_traits>
#include <vector>

const struct Take {
    template <class C>
    auto operator()(std::size_t upto, const C& c) const {
        using X = std::decay_t<decltype(*std::begin(c))>;
        std::vector<X> r;
        const std::size_t n = std::min(upto, count(c));
        r.reserve(n);
        auto it = std::begin(c);
        for (std::size_t i = 0; i < n; ++i, ++it) r.push_back(*it);
        return r;
    }
} take;
