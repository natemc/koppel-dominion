#include <deck.h>
#include <algorithm>
#include <append.h>
#include <concat.h>
#include <each.h>
#include <find.h>
#include <iostream>
#include <iterator>
#include <lambda.h>
#include <over.h>
#include <print_card.h>
#include <sum.h>

Deck::Deck(Random* r): gen(r) {
}

void Deck::discard(const Card* card) {
    discard_from(hand, card);
}

void Deck::discard(const Cards& cards) {
    discard_from(hand, cards);
}

void Deck::discard_from(Cards& from, const Card* card) {
    trash_from(from, card);
    discard_pile.push_back(card);
}

void Deck::discard_from(Cards& from, const Cards& cards) {
    for (auto& c: cards) discard_from(from, c);
}

void Deck::discard_from_draw_pile(const Card* card) {
    discard_from(draw_pile, card);
}

void Deck::discard_from_draw_pile(const Cards& cards) {
    discard_from(draw_pile, cards);
}

void Deck::draw(int n) {
    assert(0 <= n);

    for (; n && draw_pile.size(); --n, draw_pile.pop_back())
        hand.push_back(draw_pile.back());
    if (draw_pile.empty()) {
        draw_pile = gen->shuffle(discard_pile);
        discard_pile.clear();
        if (n && draw_pile.size()) draw(n);
    }
}

void Deck::end_turn() {
    draw_pile = gen->shuffle(whole());
    discard_pile.clear();
    hand.clear();
    in_play.clear();
    draw(5);
}

const Card* Deck::next() {
    if (draw_pile.empty()) {
        if (discard_pile.empty()) return nullptr;
        draw_pile = gen->shuffle(discard_pile);
        discard_pile.clear();
    }
    const Card* const card = draw_pile.back();
    draw_pile.pop_back();
    return card;
}

void Deck::play(const Card* card) {
    trash_from(hand, card);
    in_play.push_back(card);
}

void Deck::put_on_top(const Card* card) {
    draw_pile.push_back(card);
}

void Deck::put_on_top(const Cards& cards) {
    append(draw_pile, cards);
}

void Deck::trash(const Card* card) {
    trash_from(hand, card);
}

void Deck::trash(const Cards& cards) {
    trash_from(hand, cards);
}

void Deck::trash_from(Cards& from, const Card* card) {
    const auto it = find(from, card);
    assert(it != std::end(from));
    from.erase(it);
}

void Deck::trash_from(Cards& from, const Cards& cards) {
    for (auto& c: cards) trash_from(from, c);
}

void Deck::trash_from_draw_pile(const Card* card) {
    trash_from(draw_pile, card);
}

void Deck::trash_from_draw_pile(const Cards& cards) {
    trash_from(draw_pile, cards);
}

std::size_t Deck::size() const {
    return whole().size();
}

int Deck::victory_points() const {
    return sum(each(L1(x->victory_points(*this)), whole()));
}

Deck::Cards Deck::whole() const {
    return over(concat, {aside, draw_pile, discard_pile, hand, in_play});
}

std::ostream& operator<<(std::ostream& os, const Deck& deck) {
    print_cards(os << "Hand: "   , deck.hand        , deck) << "; ";
    print_cards(os << "Aside: "  , deck.aside       , deck) << "; ";
    print_cards(os << "In Play: ", deck.in_play     , deck) << "; ";
    print_cards(os << "Draw: "   , deck.draw_pile   , deck) << "; ";
    print_cards(os << "Discard: ", deck.discard_pile, deck);
    return os;
}
