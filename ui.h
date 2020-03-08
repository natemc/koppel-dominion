#pragma once

////////////////////////////////////////////////////////////////////////////////
// This interface was backed out from an early game implementation.  It
// implements the methods needed so that no UI interactions appear in any of
// the non-UI files.  There is an unresolved smell with the choice functions
// for human users; so far all the ideas I've had to fix this have been
// terrible.
////////////////////////////////////////////////////////////////////////////////

#include <vector>

struct Card;
struct Game;
struct Player;

struct UI {
    using Cards = std::vector<const Card*>;

    virtual             ~UI            ()                                         = default;
    virtual void        begin_game     (const Game&)                              = 0;
    virtual void        begin_turn     (const Game&, const Player&)               = 0;
    virtual void        buy            (const Game&, const Player&, const Card& ) = 0;
    virtual const Card* choose_action  (const Game&)                              = 0;
    virtual void        complete_action(const Game&)                              = 0;
    virtual const Card* choose_buy     (const Game&, int coins)                   = 0;
    virtual void        end_game       (const Game&)                              = 0;
    virtual void        gains          (const Game&, const Player&, const Card& ) = 0;
    virtual void        no_more        (const Game&, const Player&, const Card& ) = 0;
    virtual void        play           (const Game&, const Player&, const Card& ) = 0;
    virtual void        react          (const Game&, const Player&, const Card& ) = 0;
    virtual void        show_cards     (const Game&, const Player&, const Cards&) = 0;
    virtual void        trash          (const Game&, const Player&, const Cards&) = 0;
};
