#pragma once

////////////////////////////////////////////////////////////////////////////////
// A Tag is an enum used to make decisions regarding Cards.  Most Cards have
// only a single Tag, but any combination may someday appear for some
// yet-unimagined Card.
//
// (END_TAGS is not a Tag in the sense that it is meaningless in the game, and
// it cannot be applied to a Card; it is used for iteration and error checking).
////////////////////////////////////////////////////////////////////////////////

#include <iosfwd>

enum Tag {
    ACTION, TREASURE, VICTORY, ATTACK, REACTION, CURSE, END_TAGS
};

std::ostream& operator<<(std::ostream& os, Tag t);
