#pragma once

////////////////////////////////////////////////////////////////////////////////
// A Card has
//   an action (for some cards, this is a nop; see tags below)
//   a  name
//   a  cost
//   a  treasure point value
//   a  function to compute its victory point value given the deck
//     of the Player holding this Card
//   a  set of tags
//      A Card is usually an ACTION, TREASURE, or VICTORY Card; however, these
//      categories overlap, and there are finer distinctions that are
//      occasionally relevant to the outcome of play.  Thus a Card has tags
//      instead of having a category.  If a Card has the ACTION tag, a Player
//      (with a positive number of action points remaining) holding the Card in
//      his or her hand can play the Card during the ACTION phase of his or her
//      turn.  Otherwise, it is a programming error to invoke the perform
//      method of the Card's action member.
////////////////////////////////////////////////////////////////////////////////

#include <action.h>
#include <bitset>
#include <functional>
#include <initializer_list>
#include <iosfwd>
#include <string>
#include <tag.h>

struct Deck;

struct Card {
    using vpcalc = std::function<int(const Deck&)>;

    Card(Action a, const char* n, int c, int tp, int vp,
         std::initializer_list<Tag> tg);
    Card(Action a, const char* n, int c, int tp, vpcalc vp,
         std::initializer_list<Tag> tg);
    Card(Action a, const char* n, int c, int tp, int vp,
         std::initializer_list<Tag> tg, Action react);
    Card(Action a, const char* n, int c, int tp, vpcalc vp,
         std::initializer_list<Tag> tg, Action react);

    bool is              (Tag t)          const;
    void perform_action  (Game&, Player&) const;
    void perform_reaction(Game&, Player&) const;

    Action                 action;
    std::string            name;
    int                    cost;
    std::bitset<END_TAGS>  tags;
    int                    treasure_points;
    vpcalc                 victory_points;
    Action                 reaction;
};

std::ostream& operator<<(std::ostream& os, const Card& c);
