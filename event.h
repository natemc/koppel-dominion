#pragma once

#include <functional>
#include <iosfwd>
#include <string>

struct Game;

struct Event {
    std::string                      description;
    std::function<bool(const Game&)> expire;
    std::function<bool(const Game&)> trigger;
    std::function<void(Game&)>       fire;
};

std::ostream& operator<<(std::ostream& os, const Event&);
