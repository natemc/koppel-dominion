#pragma once

////////////////////////////////////////////////////////////////////////////////
// fold left
////////////////////////////////////////////////////////////////////////////////

#include <cassert>
#include <count.h>
#include <initializer_list>
#include <iterator>
#include <numeric>
#include <utility>

const struct Over {
    template <class F, class I, class C>
    auto operator()(F&& f, I&& init, const C& x) const {
        return std::accumulate(std::begin(x), std::end(x),
                               std::forward<I>(init), std::forward<F>(f));
    }

    template <class F, class C>
    auto operator()(F&& f, const C& x) const {
        return go(std::forward<F>(f), x);
    }

    template <class F, class I, class X>
    auto operator()(F&& f, I&& init, std::initializer_list<X> x) const {
        return std::accumulate(std::begin(x), std::end(x),
                               std::forward<I>(init), std::forward<F>(f));
    }

    template <class F, class X>
    auto operator()(F&& f, std::initializer_list<X> x) const {
        return go(std::forward<F>(f), x);
    }

private:
    template <class F, class C>
    auto go(F&& f, const C& x) const {
        assert(0 < count(x));
        auto it = std::begin(x);
        const auto init = *it;
        return std::accumulate(++it, std::end(x), init, std::forward<F>(f));
    }
} over;
