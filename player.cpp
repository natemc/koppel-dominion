#include <player.h>
#include <iostream>

Player::Player(std::string n_, Deck deck_, action_chooser_t ac, card_chooser_t cc, UI* ui_):
    name(std::move(n_)), deck(std::move(deck_)), action_chooser(std::move(ac)),
    card_chooser(std::move(cc)), ui(ui_)
{
    deck.end_turn();
}

const Action& Player::choose_action(const Game& g, const Actions& actions) const {
    return action_chooser(g, *this, actions);
}

const Card* Player::choose_card(const Game& g, const std::vector<const Card*>& cards) const
{
    auto v = card_chooser(g, *this, cards, 0u, 1u);
    assert(v.size() <= 1);
    return v.empty()? nullptr : v[0];
}

std::vector<const Card*>
Player::choose_cards
(const Game& g, const std::vector<const Card*>& cards, std::size_t how_many) const
{
    return card_chooser(g, *this, cards, 0u, how_many);
}

std::vector<const Card*>
Player::choose_cards(const Game& g, const Cards& cards) const {
    return choose_cards(g, cards, cards.size());
}

std::vector<const Card*>
Player::choose_exactly
(const Game& g, const std::vector<const Card*>& cards, std::size_t how_many) const
{
    return card_chooser(g, *this, cards, how_many, how_many);
}

int Player::victory_points() const {
    return deck.victory_points();
}

std::ostream& operator<<(std::ostream& os, Player::mat_t mat) {
    switch (mat) {
    case Player::NATIVE_VILLAGE: return os << "Native Village";
    case Player::PIRATE_SHIP   : return os << "Pirate Ship";
    }
}

std::ostream& operator<<(std::ostream& os, const Player& p) {
    os << p.name << "    " << p.deck << '\n';
    for (auto& m: p.mats)
        os << "    " << m.first << " mat: " << m.second << '\n';
    return os;
}
