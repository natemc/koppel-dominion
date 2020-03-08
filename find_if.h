#pragma once

////////////////////////////////////////////////////////////////////////////////
// The iterator pointing to the first element of the given sequence that
// satisfies the given predicate or, if no such element is found, the iterator
// std::end(that sequence)
////////////////////////////////////////////////////////////////////////////////

#include <algorithm>
#include <iterator>
#include <utility>

const struct FindIf {
    template <class F, class C>
    auto operator()(F&& f, C& c) const {
        return std::find_if(std::begin(c), std::end(c), std::forward<F>(f));
    }
} find_if;
