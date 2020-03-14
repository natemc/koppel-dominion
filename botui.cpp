#include <botui.h>
#include <filter.h>
#include <game.h>
#include <lambda.h>
#include <player.h>

void BotUI::begin_game(const Game& g) {
}

void BotUI::begin_turn(const Game& g, const Player& p) {
}

void BotUI::buy(const Game&, const Player&, const Card&) {
}

const Card* BotUI::choose_action(const Game& g) {
    const Player& p = *g.turn.player;
    return p.choose_card(g, filter(L1(x->is(ACTION)), p.deck.hand));
}

void BotUI::complete_action(const Game& g) {
}

const Card* BotUI::choose_buy(const Game& g, int coins) {
    return g.turn.player->choose_card(g, g.affordable(coins));
}

void BotUI::end_game(const Game& g) {
}

void BotUI::gains(const Game& g, const Player& p, const Card& c) {
}

void BotUI::no_more(const Game&, const Player&, const Card&) {
}

void BotUI::notify(const Game&, const Player&, const std::string&) {
}

void BotUI::play(const Game&, const Player&, const Card&) {
}

void BotUI::react(const Game& g, const Player& p, const Card& blocker) {
}

void BotUI::show_cards(const Game& g, const Player& p, const Cards& cards) {
}

void BotUI::trash(const Game& g, const Player& p, const Cards& cards) {
}
