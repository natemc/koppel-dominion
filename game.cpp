#include <game.h>
#include <count.h>
#include <each.h>
#include <filter.h>
#include <find_if.h>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <lambda.h>
#include <max.h>
#include <utility>

Game::Game(std::vector<Player*>       players_,
           std::map<const Card*, int> piles_):
    players(std::move(players_)), piles(std::move(piles_))
{
}

std::vector<const Card*> Game::affordable(int coins) const {
    return each(L1(x.first),
                filter(L1(x.first->cost <= coins && 0 < x.second),
                       piles));
}

void Game::init_turn(Player& p) {
    turn.actions = 1;
    turn.buys    = 1;
    turn.coins   = 0;
    turn.phase   = Turn::ACTION;
    turn.played  = nullptr;
    turn.player  = &p;
}

void Game::remove_card_from_piles(const Card* card) {
    auto it = piles.find(card);
    assert(it != std::end(piles) && 0 < it->second);

    // We need to retain the pile with a count of zero, because certain cards
    // (e.g., Poacher) depend on it.
    --it->second;
}

std::ostream& operator<<(std::ostream& os, const Game& g) {
    if (g.piles.empty())
        os << "No piles";
    else {
        os << "Piles: ";
        auto it = std::begin(g.piles);
        os << it->second << ' ' << *it->first;
        for (++it; it != std::end(g.piles); ++it)
            os << ',' << it->second << ' ' << *it->first;
    }
    os << '\n';
    os << "Players:\n";
    const std::size_t name_width = max(each(count, each(L1(x->name), g.players)));
    for (auto& p: g.players) {
        os << std::left << std::setw(name_width) << p->name << std::right << ": "
           << p->deck << '\n';
        for (auto m: p->mats)
            os << "    " << m.second << ": " << m.first << '\n';
    }
    return os << "Current turn: " << g.turn << '\n';
}
