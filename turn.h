#pragma once

////////////////////////////////////////////////////////////////////////////////
// A Turn has
//   a number of actions remaining
//   a number of buys remaining
//   a number of coins remaining
//   a phase, either ACTION or BUY
//   the last Card played by the current Player
//   the current Player, i.e., the Player whose turn it is and whose turn
//     is described by this Turn's state
////////////////////////////////////////////////////////////////////////////////

#include <iosfwd>

struct Card;
struct Player;

struct Turn {
    int                        actions;
    int                        buys;
    int                        coins;
    int                        count;
    enum Phase { ACTION, BUY } phase;
    const Card*                played;
    Player*                    player;
};

std::ostream& operator<<(std::ostream& os, const Turn& turn );
std::ostream& operator<<(std::ostream& os, Turn::Phase phase);
