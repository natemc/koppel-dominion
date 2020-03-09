#include <card.h>
#include <deck.h>
#include <each.h>
#include <filter.h>
#include <iostream>
#include <k.h>
#include <lambda.h>
#include <print_card.h>
#include <random.h>
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
    // print_card requires a Deck (which requires a Random, which requires a
    // standard-conforming random engine) because of the Gardens card.

    static struct UnrandomEngine {
        using result_type = std::size_t;
        result_type operator()() { return 42; }
        static constexpr result_type max() { return 42; }
        static constexpr result_type min() { return 42; }
    } not_random;
    static Random bogus{not_random};
    return print_card(os, c, Deck{&bogus});
}
