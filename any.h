#pragma once

#include <algorithm>
#include <iterator>

const struct Any {
    template <class P, class C> bool operator()(P&& pred, const C& c) const {
        return std::any_of(std::begin(c), std::end(c), std::forward<P>(pred));
    }
} any;
