#pragma once

////////////////////////////////////////////////////////////////////////////////
// Given a sequence, creates a map from the distinct values in that sequence
// to the positions where those values appear in that sequence.
// For example,
//   group({0,0,2,1,2,1,2,3,2,1}) ->
//     0: {0,1}
//     1: {3,5,9}
//     2: {2,4,6,8}
//     3: {7}
//
// Since group returns an std::map, the items in the sequence must be orderable
// by <.  Moreover, iterating over group's result will present the groups in
// item order per < (as shown in the example above).
////////////////////////////////////////////////////////////////////////////////

#include <cstddef>
#include <iterator>
#include <map>
#include <type_traits>
#include <vector>

const struct Group {
    template <class C> auto operator()(const C& x) const {
        using X = std::decay_t<decltype(*std::begin(x))>;
        std::map<X, std::vector<std::size_t>> r;
        std::size_t i = 0;
        for (const auto& e: x) r[e].push_back(i++);
        return r;
    }
} group;
