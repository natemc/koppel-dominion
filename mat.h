#pragma once

////////////////////////////////////////////////////////////////////////////////
// A Mat has
//   a set of Cards
//   a number of coin_tokens
// The contents of mats are manipulated by action Cards.
////////////////////////////////////////////////////////////////////////////////

#include <card.h>
#include <iosfwd>
#include <vector>

struct Mat {
    std::vector<const Card*> cards;
    int                      coin_tokens;
};

std::ostream& operator<<(std::ostream& os, const Mat& mat);
