#pragma once

#include <filter.h>
#include <find.h>
#include <lambda.h>
#include <utility>
#include <vector>

const struct Except {
    template <class X, class Y>
    auto operator()(X&& lhs, const Y& rhs) const {
        return filter(L1(x == rhs), std::forward<X>(lhs));
    }

    template <class X, class Y>
    auto operator()(X&& lhs, const std::vector<Y>& rhs) const {
        return filter(L1(find(rhs, x) == std::end(rhs)), std::forward<X>(lhs));
    }
} except;

template <class X> struct BoundExcept { X x; };

template <class X>
auto operator^(X x, const Except&) { return BoundExcept<X>{std::move(x)}; }

template <class X, class Y> auto operator^(BoundExcept<X> x, Y&& y) {
    return except(std::move(x.x), std::forward<Y>(y));
}
