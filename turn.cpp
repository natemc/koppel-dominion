#include <turn.h>
#include <card.h>
#include <iostream>
#include <player.h>

std::ostream& operator<<(std::ostream& os, const Turn& turn) {
    os << "    " << turn.player->name << "'s turn (" << turn.phase << " phase) "
       << turn.actions << " actions, " << turn.buys << " buys, "
       << turn.coins << " coins";
    if (turn.played) os << "; just played " << *turn.played;
    return os;
}

std::ostream& operator<<(std::ostream& os, Turn::Phase phase) {
    switch (phase) {
    case Turn::ACTION: return os << "action";
    case Turn::BUY   : return os << "buy";
    }
}
