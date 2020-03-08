#pragma once

////////////////////////////////////////////////////////////////////////////////
// A Player has
//    a brain
//      The brain is how the Player makes choices.  At the moment, there are
//      only two kinds of choices supported:
//        choosing a subset from a given set of cards
//        choosing a single action from a given set of actions
//      This is simple enough I put the choosers in Player rather than
//      introducing a Brain type.
//    a deck of cards
//    a set of mats where (when certain cards are played)
//      the Player can stash cards or coin tokens
//
// To support interaction, each Player as also a UI, but I'm not convinced this
// is a good design.  It seems weird, since human players make choices via this
// UI.  I'd like to write a client-server version and see how things shake out.
////////////////////////////////////////////////////////////////////////////////

#include <card.h>
#include <cstddef>
#include <deck.h>
#include <functional>
#include <iosfwd>
#include <map>
#include <mat.h>
#include <string>
#include <vector>

struct Game;
struct UI;

struct Player {
    using Actions          = std::vector<Action>;
    using action_chooser_t = std::function<const Action&(const Game&,
                                                         const Player&,
                                                         const Actions&)>;
    using Cards            = std::vector<const Card*>;
    using card_chooser_t   = std::function<Cards(const Game&,
                                                 const Player&,
                                                 const Cards&,
                                                 std::size_t,   // at least
                                                 std::size_t)>; // at most
    enum mat_t { NATIVE_VILLAGE, PIRATE_SHIP };

    Player(std::string n_, Deck deck_, action_chooser_t ac, card_chooser_t cc, UI* ui_);

    const Action& choose_action (const Game& g, const Actions& actions)                   const;
    const Card*   choose_card   (const Game& g, const Cards& cards)                       const;
    Cards         choose_cards  (const Game& g, const Cards& cards) const;
    Cards         choose_cards  (const Game& g, const Cards& cards, std::size_t how_many) const;
    Cards         choose_exactly(const Game& g, const Cards& cards, std::size_t how_many) const;

    int victory_points() const;

    std::string          name;
    Deck                 deck;
    std::map<mat_t, Mat> mats;
    action_chooser_t     action_chooser;
    card_chooser_t       card_chooser;
    UI*                  ui;
};

std::ostream& operator<<(std::ostream& os, Player::mat_t mat);
std::ostream& operator<<(std::ostream& os, const Player& p);
