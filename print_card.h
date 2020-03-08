#pragma once

////////////////////////////////////////////////////////////////////////////////
// You have to pass the corresponding Deck because a card's VP value may depend
// on it.
////////////////////////////////////////////////////////////////////////////////

#include <iosfwd>
#include <vector>

struct Card;
struct Deck;

std::ostream& print_card        (std::ostream& os, const Card& c, const Deck& d);
std::ostream& print_card_details(std::ostream& os, const Card& c, const Deck& d);

std::ostream& print_cards(std::ostream& os, const std::vector<const Card*>& cards, const Deck& d);
