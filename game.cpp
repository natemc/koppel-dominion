#include <game.h>
#include <algorithm>
#include <count.h>
#include <each.h>
#include <filter.h>
#include <find_if.h>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <lambda.h>
#include <max.h>
#include <ui.h>
#include <utility>

Game::Game(std::vector<Player*>       players_,
           std::map<const Card*, int> piles_):
    players(std::move(players_)), piles(std::move(piles_))
{
    turn.count = 0;
}

std::vector<const Card*> Game::affordable(int coins) const {
    return each(L1(x.first),
                filter(L1(x.first->cost <= coins && 0 < x.second),
                       piles));
}

void Game::init_turn(Player& p) {
    ++turn.count;
    turn.actions = 1;
    turn.buys    = 1;
    turn.coins   = 0;
    turn.phase   = Turn::ACTION;
    turn.played  = nullptr;
    turn.player  = &p;
}

void Game::process_events() {
    events.erase(std::remove_if(events.begin(), events.end(), L1(x.expire(*this))),
                 events.end());
    auto to_fire = filter(L1(x.trigger(*this)), events);
    events.erase(std::remove_if(events.begin(), events.end(), L1(x.trigger(*this))),
                 events.end());
    for (Event& e: to_fire) {
        for (auto& p: players) p->ui->notify(*this, *turn.player, e.description);
        e.fire(*this);
    }
}

void Game::remove_card_from_piles(const Card* card) {
    auto it = piles.find(card);
    assert(it != std::end(piles) && 0 < it->second);

    // We need to retain the pile with a count of zero, because certain cards
    // (e.g., Poacher) depend on it.
    --it->second;
}

void Game::schedule(Event e) {
    events.push_back(std::move(e));
}

void Game::schedule(std::vector<Event> e) {
    events.reserve(events.size() + e.size());
    std::copy(std::make_move_iterator(std::begin(e)),
              std::make_move_iterator(std::end  (e)),
              std::back_inserter(events));
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
