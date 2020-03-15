#include <action.h>
#include <algorithm>
#include <cassert>
#include <except.h>
#include <filter.h>
#include <game.h>
#include <iostream>
#include <iterator>
#include <join.h>
#include <lambda.h>
#include <player.h>
#include <sstream>
#include <sum.h>
#include <vector>

Action::Action(std::string desc, std::function<void(Game&, Player&)> perf):
    Action(std::move(desc),
           [go{std::move(perf)}](Game& g, Player& p, const Card*){ go(g, p); })
{
}

Action::Action(std::string desc,
               std::function<void(Game&, Player&, const Card*)> perf):
    description(std::move(desc)), perform(std::move(perf))
{
}

void Action::operator()(Game& g, Player& p, const Card* c) const {
    perform(g, p, c);
}

Action choice_action(std::initializer_list<Action> alist) {
    assert(0 < alist.size());

    std::ostringstream os;
    os << "Choose one: " << join("; or ", alist);
    std::vector<Action> actions(alist);
    return Action{os.str(), [=](Game& g, Player& p, const Card* c) {
                      p.choose_action(g, actions)(g, p, c);
                  }};
}

Action sequence_action(std::initializer_list<Action> alist) {
    assert(0 < alist.size());

    std::vector<Action> actions(alist);
    return Action{join("; ", alist), [=](Game& g, Player& p, const Card* c) {
                      for (auto& a: actions) a(g, p, c);
                  }};
}

std::ostream& operator<<(std::ostream& os, const Action& a) {
    return os << a.description;
}
