#pragma once

////////////////////////////////////////////////////////////////////////////////
// The key (i.e., domain) of a map
////////////////////////////////////////////////////////////////////////////////

#include <algorithm>
#include <iterator>
#include <lambda.h>
#include <map>
#include <vector>

const struct Key {
    template <class K, class V>
    auto operator()(const std::map<K, V>& m) const {
        std::vector<K> r;
        r.reserve(m.size());
        std::transform(std::begin(m), std::end(m), std::back_inserter(r), L1(x.first));
        return r;
    }
} key;
