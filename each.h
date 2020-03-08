#pragma once

////////////////////////////////////////////////////////////////////////////////
// Apply a function to each element of a container.
// Supports the following overloads:
//   (f, seq<a>  ) -> vector<f(a)>
//   (f, map<k,v>) -> map   <k,f(v)>
////////////////////////////////////////////////////////////////////////////////

#include <algorithm>
#include <count.h>
#include <initializer_list>
#include <iterator>
#include <map>
#include <type_traits>
#include <utility>
#include <vector>

const struct Each {
    template <class F, class C>
    auto operator()(F&& f, C& x) const {
        using R = std::decay_t<decltype(f(*std::begin(x)))>;
        std::vector<R> r;
        r.reserve(count(x));
        std::transform(std::begin(x), std::end(x), std::back_inserter(r),
                       std::forward<F>(f));
        return r;
    }

    template <class F, class C>
    auto operator()(F&& f, const C& x) const {
        using R = std::decay_t<decltype(f(*std::begin(x)))>;
        std::vector<R> r;
        r.reserve(count(x));
        std::transform(std::begin(x), std::end(x), std::back_inserter(r),
                       std::forward<F>(f));
        return r;
    }

    template <class F, class X>
    auto operator()(F&& f, std::initializer_list<X> x) const {
        using R = std::decay_t<decltype(f(*std::begin(x)))>;
        std::vector<R> r;
        r.reserve(count(x));
        std::transform(std::begin(x), std::end(x), std::back_inserter(r),
                       std::forward<F>(f));
        return r;
    }

    template <class F, class K, class V>
    auto operator()(F&& f, std::map<K,V>& x) const {
        using R = std::decay_t<decltype(f(std::begin(x)->second))>;
        std::map<K,R> r;
        for (auto& e: x) r.insert({e.first, f(e.second)});
        return r;
    }

    template <class F, class K, class V>
    auto operator()(F&& f, const std::map<K,V>& x) const {
        using R = std::decay_t<decltype(f(std::begin(x)->second))>;
        std::map<K,R> r;
        for (auto& e: x) r.insert({e.first, f(e.second)});
        return r;
    }
} each;
