#pragma once

////////////////////////////////////////////////////////////////////////////////
// Add the elements of one sequence to the end of another.
////////////////////////////////////////////////////////////////////////////////

#include <initializer_list>
#include <iterator>

const struct Append {
    template <class X, class Y>
    void operator()(X& x, const Y& y) const {
        x.insert(std::end(x), std::begin(y), std::end(y));
    }

    template <class X, class Y>
    void operator()(X& x, std::initializer_list<Y> y) const {
        x.insert(std::end(x), std::begin(y), std::end(y));
    }
} append;
