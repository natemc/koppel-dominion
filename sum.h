#pragma once

////////////////////////////////////////////////////////////////////////////////
// The sum of the elements of a sequence
////////////////////////////////////////////////////////////////////////////////

#include <functional>
#include <iterator>
#include <over.h>
#include <type_traits>

const struct Sum {
    template <class C>
    auto operator()(const C& x) const {
        using X = std::decay_t<decltype(*std::begin(x))>;
        return over(std::plus<X>{}, X(0), x);
    }
} sum;
