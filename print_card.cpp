#include <print_card.h>
#include <card.h>
#include <count.h>
#include <deck.h>
#include <each.h>
#include <filter.h>
#include <group.h>
#include <iostream>
#include <iterator>

std::ostream& print_card(std::ostream& os, const Card& c, const Deck& d) {
    os << c.name << ' ';
    for (Tag t: filter(L1(c.is(x)), each(L1(Tag(x)), til(END_TAGS))))
        os << t;
    return os << " C" << c.cost << "|T" << c.treasure_points
              << "|V" << c.victory_points(d);
}

std::ostream& print_card_details(std::ostream& os, const Card& c, const Deck& d) {
    print_card(os, c, d);
    if (c.is(ACTION  )) os << ' '      << c.action  .description;
    if (c.is(REACTION)) os << " ---- " << c.reaction.description;
    return os;
}

std::ostream&
print_cards(std::ostream& os, const std::vector<const Card*>& cards, const Deck& d)
{
    auto m = each(count, group(cards));
    auto it = std::begin(m);
    if (it != std::end(m)) {
        print_card(os << it->second << ' ', *it->first, d);
        for (++it; it != std::end(m); ++it)
            print_card(os << ", " << it->second << ' ', *it->first, d);
    }
    return os;
}
