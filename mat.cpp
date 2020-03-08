#include <mat.h>
#include <card.h>
#include <count.h>
#include <each.h>
#include <group.h>
#include <iostream>
#include <iterator>

std::ostream& operator<<(std::ostream& os, const Mat& mat) {
    if (mat.cards.empty())
        os << "0 cards";
    else {
        auto m = each(count, group(mat.cards));
        auto it = std::begin(m);
        os << it->second << ' ' << *it->first;
        for (++it; it != std::end(m); ++it)
            os << ", " << it->second << ' ' << *it->first;
    }
    return os << "; " << mat.coin_tokens << " coin tokens";
}
