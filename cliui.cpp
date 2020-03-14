#include <cliui.h>
#include <algorithm>
#include <each.h>
#include <filter.h>
#include <game.h>
#include <group.h>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <lambda.h>
#include <max.h>
#include <player.h>
#include <print_card.h>
#include <sort.h>
#include <sstream>
#include <vector>

namespace {
    struct ActionChooser {
        ActionChooser(std::istream& in, std::ostream& out): is(in), os(out) {}

        const Action& operator()(const Game&                game,
                                 const Player&              player,
                                 const std::vector<Action>& actions);
    
        std::istream& is;
        std::ostream& os;
    };

    const Action& ActionChooser::operator()(const Game&                game,
                                            const Player&              player,
                                            const std::vector<Action>& actions)
    {
        assert(0 < actions.size());
        if (actions.size() == 1) return actions[0];
        os << "    Choose one action to perform:\n";
        for (std::size_t i = 0; i < actions.size(); ++i)
            os << "        " << i << '|' << actions[i] << '\n';
        os << "    Selection: " << std::flush;
        std::string line;
        for (;;) {
            if (!std::getline(is, line)) {
                std::cerr << "Input error\n";
                exit(1);
            }
            std::istringstream is(line);
            std::size_t n;
            if ((is >> n) && n < actions.size()) return actions[n];
            os << "    ! Please enter 0-" << (actions.size()-1)
               << ": " << std::flush;
        }
        // unreachable
    }

    struct CardChooser {
        CardChooser(std::istream& in, std::ostream& out): is(in), os(out) {}

        std::vector<const Card*>
        operator()(const Game&                     game,
                   const Player&                   player,
                   const std::vector<const Card*>& cards,
                   std::size_t                     at_least,
                   std::size_t                     at_most_);

        std::istream& is;
        std::ostream& os;
    };

    std::vector<const Card*>
    CardChooser::operator()(const Game&                     game,
                            const Player&                   player,
                            const std::vector<const Card*>& cards,
                            std::size_t                     at_least,
                            std::size_t                     at_most_)
    {
        const std::size_t at_most = std::min(at_most_, cards.size());
        assert(at_least <= at_most && at_most <= cards.size());

        if (at_least == cards.size()) return cards;
        os << "    Select from " << at_least << " to " << at_most
                  << " cards [p N for details; s to skip]:\n";
        for (std::size_t i = 0; i < cards.size(); ++i)
            print_card(os << "        " << i << '|', *cards[i], player.deck)
                << '\n';
        os << "    Selection: " << std::flush;
        std::string line;
        for (;;) {
            if (!std::getline(is, line)) {
                std::cerr << "Input error\n";
                exit(1);
            }
            if (line.size()) {
                if (line[0] == 's') return std::vector<const Card*>{};
                if (line[0] == 'p') {
                    std::istringstream is(line.substr(1));
                    std::size_t n;
                    if ((is >> n) && n < cards.size())
                        print_card_details(os << "        ", *cards[n], player.deck);
                    else
                        os << "    ! Invalid selection.";
                    os << "\n    Selection: " << std::flush;
                    continue;
                }
                if (!std::isdigit(line[0])) {
                    os << "    ! Please enter 0-" << (cards.size()-1)
                       << " or s to skip: " << std::flush;
                    continue;
                }
            }
            std::istringstream is(line);
            std::vector<const Card*> r;
            std::size_t n;
            while ((is >> n) && n < cards.size()) r.push_back(cards[n]);
            if (n < cards.size() && at_least <= r.size() && r.size() <= at_most)
                return r;
            if (at_least > r.size() || r.size() > at_most)
                os << "    ! Please select from " << at_least << " to "
                   << at_most << " cards: " << std::flush;
            else
                os << "    ! Please enter 0-" << (cards.size()-1)
                   << " or s to skip: " << std::flush;
        }
        // unreachable
    }

    std::ostream& print_mat(std::ostream& os, const Mat& m, const Deck& d) {
        if (m.cards.empty()) os << "0 cards";
        else                 print_cards(os, m.cards, d);
        return os << "; " << m.coin_tokens << " coin tokens";
    }

    std::ostream& print_deck(std::ostream& os, const Deck& d) {
        return print_cards(os, d.whole(), d);
    }

    std::ostream& print_hand(std::ostream& os, const Deck& d) {
        return print_cards(os, d.hand, d);
    }
    
    std::ostream& print_piles(std::ostream& os, const Game& g) {
        auto it = std::begin(g.piles);
        os << *it->first;
        for (++it; it != std::end(g.piles); ++it) os << ',' << *it->first;
        return os;
    }

    const char dred   [] = "\033[37;48;5;88m";
    const char lblue  [] = "\033[30;48;5;195m";
    const char lgray  [] = "\033[30;48;5;254m";
    const char lgreen [] = "\033[30;48;5;193m";
    const char lorange[] = "\033[30;48;5;202m";
    const char lpurple[] = "\033[30;48;5;201m";
    const char lred   [] = "\033[30;48;5;196m";
    const char myellow[] = "\033[30;48;5;229m";
    const char reset  [] = "\033[0m";
} // namespace

CliUI::CliUI(std::istream&, std::ostream& out): os(out) {
}

void CliUI::begin_game(const Game& g) {
    assert(g.players.size());

    os << "Piles: ";
    print_piles(os, g) << '\n';
    const std::size_t name_width = max(each(count, each(L1(x->name), g.players)));
    for (auto& p: g.players) {
        os << std::left << std::setw(name_width) << p->name << std::right << ": ";
        print_deck(os, p->deck);
        os << '\n';
    }
}

// TODO Find a less hacky solution than using self here
void CliUI::begin_turn(const Game& g, const Player& self) {
    const Player& p = *g.turn.player;
    os << p.name << "'s (" << p.victory_points() << "VP) turn";
    if (&self == &p)
        print_hand(os << ": ", p.deck);
    os << '\n';
}

void CliUI::buy(const Game& g, const Player& p, const Card& c) {
    os << (c.is(VICTORY)? lorange : lblue);
    print_card(os << "    " << p.name << " buys ", c, p.deck) << reset << '\n';
}

const Card* CliUI::choose_action(const Game& g) {
    os << "  Actions: " << g.turn.actions << '\n';
    const Player& p = *g.turn.player;
    const Card* const card = p.choose_card(g, filter(L1(x->is(ACTION)), p.deck.hand));
    if (!card) os << "    Skip...\n";
    return card;
}

void CliUI::complete_action(const Game& g) {
    const Player& p = *g.turn.player;
    print_hand(os << "    New hand: ", p.deck) << '\n';
    for (auto& m: p.mats)
        print_mat(os << "    " << m.first << " mat: ", m.second, p.deck) << '\n';
}

const Card* CliUI::choose_buy(const Game& g, int coins) {
    os << "  Buys: " << g.turn.buys << "  Coins: " << coins << '\n';
    const Card* const bought = g.turn.player->choose_card(g, g.affordable(coins));
    if (!bought) os << "    Skip...\n";
    return bought;
}

void CliUI::end_game(const Game& g) {
    auto p = sort(g.players, L2(x->victory_points() > y->victory_points()));
    auto it = std::begin(p);
    const Player& w = **it;
    os << w.name << ", with " << w.victory_points()
       << " victory points, defeated\n";
    for (++it; it != std::end(p) - 1; ++it)
        os << "    " << (*it)->name << " with "
           << (*it)->victory_points() << " victory points,\n";
    os << "and " << (*it)->name << " with "
       << (*it)->victory_points() << " victory points.\n";
}

void CliUI::gains(const Game& g, const Player& p, const Card& c) {
    os << (c.is(VICTORY)? lorange : lblue);
    print_card(os << "    " << p.name << " gains a ", c, p.deck)
        << reset << '\n';
}

void CliUI::no_more(const Game& g, const Player& p, const Card& c) {
    print_card(os << dred << "    ! There are no more ", c, p.deck)
        << " cards" << reset << '\n';
}

void CliUI::notify(const Game&, const Player& p, const std::string& s) {
    os << "    " << p.name << ": " << s << '\n';
}

void CliUI::play(const Game& g, const Player& p, const Card& c) {
    if (c.is(ACTION)) os << (c.is(ATTACK)? lred : lgreen);
    print_card(os << "    " << p.name << " plays ", c, p.deck);
    if (c.action.description.size())
        os << ": " << c.action.description;
    if (c.is(ACTION)) os << reset;
    os << '\n';
}

void CliUI::react(const Game& g, const Player& p, const Card& blocker) {
    os << lgray << "    " << p.name << " blocked attack with ";
    print_card(os, blocker, p.deck) << reset << '\n';
}

void CliUI::show_cards(const Game& g, const Player& p, const Cards& cards) {
    print_cards(os << myellow << "    " << p.name << " has ", cards, p.deck)
        << reset << '\n';
}

void CliUI::trash(const Game& g, const Player& p, const Cards& cards) {
    print_cards(os << lpurple << "    " << p.name << " trashed ", cards, p.deck)
        << reset << '\n';
}

Player::action_chooser_t choose_action_cli(std::istream& is, std::ostream& os) {
    return ActionChooser(is, os);
}

Player::card_chooser_t choose_card_cli(std::istream& is, std::ostream& os) {
    return CardChooser(is, os);
}
