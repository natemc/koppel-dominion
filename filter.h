#pragma once

////////////////////////////////////////////////////////////////////////////////
// The elements of a sequence satisfying a given predicate
////////////////////////////////////////////////////////////////////////////////

#include <algorithm>
#include <iterator>
#include <type_traits>
#include <utility>
#include <vector>

const struct Filter {
    template <class F, class C>
    auto operator()(F&& f, const C& x) const {
        using X = std::decay_t<decltype(*std::begin(x))>;
        std::vector<X> r;
        std::copy_if(std::begin(x), std::end(x),
                     std::back_inserter(r), std::forward<F>(f));
        return r;
    }
} filter;
