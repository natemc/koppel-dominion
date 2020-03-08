#pragma once

////////////////////////////////////////////////////////////////////////////////
// The value (i.e., range) of a map
////////////////////////////////////////////////////////////////////////////////

#include <algorithm>
#include <iterator>
#include <lambda.h>
#include <map>
#include <vector>

const struct Value {
    template <class K, class V>
    auto operator()(const std::map<K, V>& m) const {
        std::vector<V> r;
        r.reserve(m.size());
        std::transform(std::begin(m), std::end(m), std::back_inserter(r), L1(x.second));
        return r;
    }
} value;
