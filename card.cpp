#include <card.h>
#include <each.h>
#include <filter.h>
#include <iostream>
#include <k.h>
#include <lambda.h>
#include <til.h>
#include <utility>

Card::Card(Action a, const char* n, int c, int tp, int vp,
           std::initializer_list<Tag> tg):
    Card(std::move(a), n, c, tp, K(vp), tg)
{}

Card::Card(Action a, const char* n, int c, int tp, vpcalc vp,
           std::initializer_list<Tag> tg):
    action(std::move(a)), name(n), cost(c), treasure_points(tp),
    victory_points(std::move(vp))
{
    for (Tag t: tg) tags.set(t);
}

bool Card::is(Tag t) const {
    return tags[t];
}

std::ostream& operator<<(std::ostream& os, const Card& c) {
    os << c.name << " C" << c.cost << "|T" << c.treasure_points << ' ';
    for (Tag t: filter(L1(c.is(Tag(x))), each(L1(Tag(x)), til(END_TAGS))))
        os << t;
    return os;
}
