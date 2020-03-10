#include <action.h>
#include <algorithm>
#include <cassert>
#include <filter.h>
#include <game.h>
#include <iostream>
#include <iterator>
#include <join.h>
#include <player.h>
#include <sstream>
#include <sum.h>
#include <ui.h>
#include <vector>

Action choice_action(std::initializer_list<Action> alist) {
    assert(0 < alist.size());

    std::ostringstream os;
    os << "Choose one: " << join("; or ", alist);
    std::vector<Action> actions(alist);
    return Action{os.str(), [=](Game& g, Player& p) {
                      p.choose_action(g, actions).perform(g, p);
                  }};
}

Action sequence_action(std::initializer_list<Action> alist) {
    assert(0 < alist.size());

    std::vector<Action> actions(alist);
    return Action{join("; ", alist), [=](Game& g, Player& p) {
                      for (auto& a: actions) a.perform(g, p);
                  }};
}

Action make_others(Action action) {
    std::ostringstream os;
    os << "Each other player " << action.description;
    return Action{os.str(), [=](Game& g, Player& p) {
        for (auto& o: filter(L1(x != &p), g.players)) {
            auto reactions = filter(L1(x->is(REACTION)), o->deck.hand);
            auto blocker = reactions.empty()? nullptr : o->choose_card(g, reactions);
            if (blocker) for (auto q: g.players) q->ui->react(g, *o, *blocker);
            else         action.perform(g, *o);
        }
    }};
}

std::ostream& operator<<(std::ostream& os, const Action& a) {
    return os << a.description;
}
