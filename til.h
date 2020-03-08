#pragma once

////////////////////////////////////////////////////////////////////////////////
// The integers 0, 1, ..., n-1
////////////////////////////////////////////////////////////////////////////////

#include <cstddef>
#include <iterator>
#include <numeric>
#include <vector>

const struct Til {
    std::vector<std::size_t> operator()(std::size_t n) const {
        std::vector<std::size_t> r(n);
        std::iota(std::begin(r), std::end(r), std::size_t(0));
        return r;
    }
} til;
