#pragma once

#include <functional>
#include <iosfwd>
#include <string>

struct Card;
struct Game;

struct Event {
    // Whence this event?  Does this mean we need to distinguish
    // instances of the same card?  That should be possible, e.g.,
    // using Pile = std::vector<Card> and then continue to work
    // with const Card* everywhere.
    const Card*                      card;
    std::string                      description;
    std::function<bool(const Game&)> expire;
    std::function<bool(const Game&)> trigger;
    std::function<void(Game&)>       fire;
};

std::ostream& operator<<(std::ostream& os, const Event&);
