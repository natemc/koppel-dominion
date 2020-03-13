#pragma once

////////////////////////////////////////////////////////////////////////////////
// A Game is
//   a set of Players
//   a set of piles of purchasable Cards
//   the state of the current Turn
////////////////////////////////////////////////////////////////////////////////

#include <card.h>
#include <iosfwd>
#include <map>
#include <player.h>
#include <turn.h>
#include <vector>

struct Game {
    Game(std::vector<Player*>       players_,
         std::map<const Card*, int> piles_);

    std::vector<const Card*> affordable() const {
        return affordable(turn.coins);
    }

    std::vector<const Card*> affordable            (int coins) const;
    void                     init_turn             (Player& p);
    void                     remove_card_from_piles(const Card*);

    std::vector<Player*>       players;
    std::map<const Card*, int> piles;
    Turn                       turn;
};

std::ostream& operator<<(std::ostream& os, const Game& game);
