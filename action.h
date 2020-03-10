#pragma once

////////////////////////////////////////////////////////////////////////////////
// An Action is
//   a description
//   a function with full access to the Game
// An Action is typically const, but does not have to be (i.e., an Action's
// function may contain its own mutable state).
// Factory functions for common use cases are provided:
//   choice_action
//   do_to_others
//   sequence_action
////////////////////////////////////////////////////////////////////////////////

#include <functional>
#include <initializer_list>
#include <iosfwd>
#include <string>

struct Game;
struct Player;

struct Action {
    std::string                         description;
    std::function<void(Game&, Player&)> perform;
};

Action choice_action  (std::initializer_list<Action> actions);
Action make_others    (Action                        action );
Action sequence_action(std::initializer_list<Action> actions);

std::ostream& operator<<(std::ostream& os, const Action& a);
