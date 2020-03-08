#pragma once

////////////////////////////////////////////////////////////////////////////////
// Generalized indexing, i.e.,
//    at({1,2,3}, 0         ) -> 1
//    at({1,2,3}, {0,2}     ) -> {1,3}
//    at({1,2,3}, {{0,2}, 1}) -> {{1,3}, 2}
// etc
////////////////////////////////////////////////////////////////////////////////

#include <each.h>
#include <initializer_list>
#include <lambda.h>
#include <vector>

const struct At {
    template <class C, class I>
    auto operator()(const C& c, const I& i) const {
        return c[i];
    }

    template <class C, class I>
    auto operator()(const C& c, const std::vector<I>& i) const {
        return each([&](auto&& j){return (*this)(c, j);}, i);
    }

    template <class C, class I>
    auto operator()(const C& c, std::initializer_list<I> i) const {
        return each([&](auto&& j){return (*this)(c, j);}, i);
    }
} at;
