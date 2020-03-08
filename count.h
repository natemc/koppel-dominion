#pragma once

////////////////////////////////////////////////////////////////////////////////
// The number of elements of a sequence; works for built-in arrays and
// C-style strings as well as standard library containers.
////////////////////////////////////////////////////////////////////////////////

#include <cstddef>

const struct Count {
    template <class C> std::size_t operator()(const C& x) const {
        return x.size();
    }

    template <class X, std::size_t N>
    std::size_t operator()(X(&)[N]) const { return N; }

    std::size_t operator()(const char* s) const;
} count;
