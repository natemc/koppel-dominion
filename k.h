#pragma once

////////////////////////////////////////////////////////////////////////////////
// The K combinator; an instance is a unary invocable that always returns the
// value with which it was constructed.  For example,
//   auto True = K(true);         // always returns true
//   auto p    = filter(True, x); // p gets a copy of x
////////////////////////////////////////////////////////////////////////////////

#include <utility>

template <class X>
struct BoundK {
    explicit BoundK(X x_): x(std::move(x_)) {}
    template <class Y>
    const X& operator()(const Y&) const { return x; }
    X x;
};

template <class X>
BoundK<X> K(X x) { return BoundK<X>(std::move(x)); }
