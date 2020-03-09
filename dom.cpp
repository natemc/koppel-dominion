#include <algorithm>
#include <append.h>
#include <botui.h>
#include <card.h>
#include <cassert>
#include <cliui.h>
#include <concat.h>
#include <count.h>
#include <cstdlib>
#include <cstddef>
#include <deck.h>
#include <each.h>
#include <filter.h>
#include <find.h>
#include <find_if.h>
#include <game.h>
#include <group.h>
#include <iostream>
#include <iterator>
#include <k.h>
#include <key.h>
#include <lambda.h>
#include <map>
#include <memory>
#include <player.h>
#include <print_card.h>
#include <random>
#include <random.h>
#include <reverse.h>
#include <sstream>
#include <string>
#include <sum.h>
#include <take.h>
#include <turn.h>
#include <value.h>
#include <vector>

////////////////////////////////////////////////////////////////////////////////
// Utility functions
////////////////////////////////////////////////////////////////////////////////

namespace {
    template <class P>
    const Card* choose_card_to_gain(Game& g, Player& p, P&& predicate) {
        auto gainable = filter(std::forward<P>(predicate),
                               filter(L1(0 < g.piles[x]), key(g.piles)));
        auto chosen = gainable.empty()? nullptr : p.choose_card(g, gainable);
        if (chosen) for (auto& o: g.players) o->ui->gains(g, p, *chosen);
        return chosen;
    }

    std::string describe_add(int n, const char* name) {
        std::ostringstream os;
        os << '+' << n << ' ' << name;
        if (1 < n) os << 's';
        return os.str();
    }

    // Not all cards that state "gain a card" are clear about where the gained
    // card goes.  This function puts the gained card on the top of the draw
    // pile.  The Bureaucrat is clear that the silver goes to the top of the
    // draw pile.  My observation is that the Witch's curse goes into the
    // discard pile.
    template <class P> // places the gained card
    Action gain_card(const Card* card, const std::string& description, P place) {
        return Action{description,
                      [=](Game& g, Player& p){
                          assert(g.piles.count(card));
                          if (!g.piles[card]) {
                              for (auto& o: g.players)
                                  o->ui->no_more(g, p, *card);
                          }
                          else {
                              --g.piles[card];
                              place(p, card);
                              for (auto& o: g.players)
                                  o->ui->gains(g, p, *card);
                          }
                      }
        };
    }

    void to_deck(Player& player, const Card* card) {
        player.deck.put_on_top(card);
    }

    void to_discard_pile(Player& player, const Card* card) {
        player.deck.discard_pile.push_back(card);
    }

    template <class P>
    Deck::Cards trash(Game& g, Player& p, P&& predicate, std::size_t how_many) {
        auto trashable = filter(std::forward<P>(predicate), p.deck.hand);
        auto trashed = trashable.empty()? trashable
                                        : p.choose_cards(g, trashable, how_many);
        if (trashed.size()) {
            p.deck.trash(trashed);
            for (auto& o: g.players) o->ui->trash(g, p, trashed);
        }
        return trashed;
    }

    template <class P>
    const Card* trash_one(Game& g, Player& p, P&& predicate) {
        Deck::Cards trashed = trash(g, p, std::forward<P>(predicate), 1);
        assert(trashed.size() <= 1);
        return trashed.empty()? nullptr : trashed[0];
    }
} // namespace

////////////////////////////////////////////////////////////////////////////////
// Actions
////////////////////////////////////////////////////////////////////////////////

Action add_actions(int n) {
    return Action{describe_add(n, "Action"),
                  [=](Game& g, Player&){ g.turn.actions += n; }};
}

Action add_buys(int n) {
    return Action{describe_add(n, "Buy"),
                  [=](Game& g, Player&){ g.turn.buys += n; }};
}

Action add_cards(int n) {
    return Action{describe_add(n, "Card"),
                  [=](Game& g, Player&){ g.turn.player->deck.draw(n); }};
}

Action add_coins(int n) {
    return Action{describe_add(n, "Coin"),
                  [=](Game& g, Player&){ g.turn.coins += n; }};
}

Action bureaucrat_action(const Card& silver) {
    return sequence_action({
        gain_card(&silver, "Gain a silver onto your deck", to_deck),
        do_to_others({
            "reveals a Victory card from their hand and puts it onto"
            " their deck (or reveals a hand with no Victory cards)",
            [](Game& g, Player& p) {
                const auto it = find_if(L1(x->is(VICTORY)), p.deck.hand);
                if (it == p.deck.hand.end())
                    for (auto o: g.players) o->ui->show_cards(g, p, p.deck.hand);
                else {
                    for (auto o: g.players) o->ui->show_cards(g, p, {*it});
                    p.deck.put_on_top(*it);
                    p.deck.hand.erase(it);
                }
            }
    })});
}

const Action cellar_action = {
    "+1 Action; Discard any number of cards, then draw that many",
    [](Game& g, Player& p) {
        ++g.turn.actions;
        const auto c = p.choose_cards(g, p.deck.hand, p.deck.hand.size());
        p.deck.discard(c);
        p.deck.draw(c.size());
    }
};

const Action council_room_action{sequence_action({
    add_cards(4),
    do_to_others({"draws a card", [](Game&, Player& p){ p.deck.draw(); }})
})};

Action descriptive_action(const std::string& description) {
    return {description, [](Game&,Player&){}};
}

const Action festival_action{sequence_action({
    add_actions(2), add_buys(1), add_coins(2)
})};

const Action gardens_action{descriptive_action(
    "Worth VP per 10 cards you have (round down)")};

const Action laboratory_action{sequence_action({
    add_cards(1), add_actions(1)
})};

const Action library_action = {
    "Draw until you have 7 cards in hand, skipping any Action cards"
    " you choose to; set those aside, discarding them afterwards",
    [](Game& g, Player& p) {
        Deck::Cards skipped;
        const Card* card;
        while (p.deck.hand.size() < 7 && (card = p.deck.next())) {
            auto c = !card->is(ACTION)? card : p.choose_card(g, {card});
            (c? p.deck.hand : skipped).push_back(card);
        }
        p.deck.put_on_top(reverse(skipped));
    }
};

const Action market_action{sequence_action({
    add_cards(1), add_actions(1), add_buys(1), add_coins(1)
})};

const Action militia_action{sequence_action({
    add_cards(2),
    do_to_others({
        "discards down to 3 cards in hand",
        [](Game& g, Player& p) {
            Deck::Cards& hand = p.deck.hand;
            if (hand.size() > 3) {
                p.deck.discard(p.choose_exactly(g, hand, hand.size() - 3));
            }
        }
    })
})};

const Action mine_action = {
    "You may trash a Treasure card from your hand and gain a Treasure"
    " card to your hand costing up to 3 Coins more than it",
    [](Game& g, Player& p) {
        if (auto trashed = trash_one(g, p, L1(x->is(TREASURE)))) {
            auto criteria = L1(x->is(TREASURE) && x->cost <= trashed->cost + 3);
            if (auto chosen = choose_card_to_gain(g, p, criteria))
                p.deck.hand.push_back(chosen);
        }
    }
};

const Action moat_action = {
    "+2 Cards; When another player plays an Attack"
    " card, you may first reveal this from your hand,"
    " to be unaffected by it.",
    [](Game& g, Player& p){ p.deck.draw(2); }
};

Action money_lender_action(const Card& copper) {
    return {
        "You may trash a Copper from your hand for +3 Coins",
        [&](Game& g, Player& p) {
            if (find(p.deck.hand, &copper) != std::end(p.deck.hand)) {
                for (auto& o: g.players) o->ui->trash(g, p, {&copper});
                p.deck.trash(&copper);
                g.turn.coins += 3;
            }
        }
    };
};

const Action native_village_top_card_to_mat = {
    "Put the top card of your deck face down on your Native Village mat"
    " (you may look at those cards at any time)",
    [](Game& g, Player& p) {
        if (const Card* const card = p.deck.next())
            p.mats[Player::NATIVE_VILLAGE].cards.push_back(card);
    }
};

const Action native_village_mat_to_hand = {
    "put all the cards from your mat into your hand",
    [](Game& g, Player& p) {
        auto& mat = p.mats[Player::NATIVE_VILLAGE].cards;
        append(p.deck.hand, mat);
        mat.clear();
    }
};

const Action native_village_action{sequence_action({
    add_cards(2),
    choice_action({native_village_top_card_to_mat,
                   native_village_mat_to_hand})
})};

const Action nop_action{"", [](Game&,Player&){}};

const Action pirate_ship_add_coins = {
    "+1 per Coin token on your Pirate Ship mat",
    [](Game& g, Player& p) {
        g.turn.coins += p.mats[Player::PIRATE_SHIP].coin_tokens;
    }
};

// It's not clear to me what the correct behavior is when one of the other
// players has only a single card in his or her draw pile.  In this
// implementation, the player in question only reveals that single card.
const Action pirate_ship_reveal = do_to_others({
    "reveals the top 2 cards of their deck",
    [](Game& g, Player& p) {
        const auto top2 = take(2, p.deck.draw_pile);
        for (auto& o: g.players) o->ui->show_cards(g, p, top2);
    }
});

// The phrase, "trashes one of those Treasures that you choose," is unclear.
// The word "those" seems to refer to the revealed cards and so I implemented
// this so that the instruction to trash only applies when a revealed card is
// also a Treasure card.
const Action pirate_ship_trash{
    "trashes one of those Treasures that you choose, and discards the rest"
    " and then if anyone trashed a Treasure you add a Coin Token to your"
    " Pirate Ship mat",
    [](Game& g, Player& p) {
        bool trash = false;
        for (auto& o: filter(L1(x != &p), g.players)) {
            auto top2      = take(2, o->deck.draw_pile);
            auto treasures = filter(L1(x->is(TREASURE)), top2);
            auto chosen    = treasures.empty()? nullptr
                                              : p.choose_card(g, treasures);
            if (!chosen)
                o->deck.discard_from_draw_pile(top2);
            else {
              trash = true;
              o->deck.trash_from_draw_pile(chosen);
              for (auto& o: g.players) o->ui->trash(g, p, {chosen});
              top2.erase(find(top2, chosen));
              o->deck.discard_from_draw_pile(top2);
            }
         }
         if (trash) ++p.mats[Player::PIRATE_SHIP].coin_tokens;
     }
};

const Action pirate_ship_action{choice_action({
    pirate_ship_add_coins,
    sequence_action({pirate_ship_reveal, pirate_ship_trash})
})};

const Action remodel_action = {
    "Trash a card from your hand; gain a card costing up to 2 Coins more than it",
    [](Game& g, Player& p) {
        if (auto trashed = trash_one(g, p, L1(true))) {
            auto criteria = L1(x->cost <= trashed->cost + 2);
            if (auto chosen = choose_card_to_gain(g, p, criteria))
                p.deck.discard_pile.push_back(chosen);
        }
    }
};

const Action throne_room_action = {
    "You may play an Action card from your hand twice",
    [](Game& g, Player& p) {
        auto actions = filter(L1(x->is(ACTION)), p.deck.hand);
        if (auto card = actions.empty()? nullptr : p.choose_card(g, actions)) {
            p.deck.discard(card);
            for (auto& o: g.players) o->ui->play(g, p, *card);
            card->action.perform(g, p);
            for (auto& o: g.players) o->ui->play(g, p, *card);
            card->action.perform(g, p);
        }
    }
};

Action trash_action(int n) {
    std::ostringstream os;
    os << "Trash up to " << n << " cards from your hand";
    return Action{os.str(), [=](Game& g, Player& p) {
            trash(g, p, K(true), n);
        }
    };
}

const Action village_action{sequence_action({add_cards(1), add_actions(2)})};

Action witch_action(const Card& curse) {
    return sequence_action({
        add_cards(2),
        do_to_others(gain_card(&curse, "gains a Curse", to_discard_pile))
    });
}

const Action workshop_action = {
    "Gain a card costing up to 4 Coins",
    [](Game& g, Player& p) {
        if (auto chosen = choose_card_to_gain(g, p, L1(x->cost <= 4)))
            p.deck.put_on_top(chosen);
    }
};

////////////////////////////////////////////////////////////////////////////////
// Cards
////////////////////////////////////////////////////////////////////////////////

int gardens_vp(const Deck& deck) { return deck.size() / 10; }

//         CARD           ACTION                       NAME           COST TP VP  TAGS
// ---------------------------------------------------------------------------------------------
const Card copper        (nop_action                 , "Copper"        , 0, 1, 0, {TREASURE});
const Card gold          (nop_action                 , "Gold"          , 6, 3, 0, {TREASURE});
const Card silver        (nop_action                 , "Silver"        , 3, 2, 0, {TREASURE});

const Card duchy         (nop_action                 , "Duchy"         , 5, 0, 3, {VICTORY});
const Card estate        (nop_action                 , "Estate"        , 2, 0, 1, {VICTORY});
const Card province      (nop_action                 , "Province"      , 8, 0, 6, {VICTORY});

// dominion.games allows you to buy a curse card and it does just what this does.
const Card curse         (nop_action                 , "Curse"         , 0, 0,-1, {CURSE});

const Card bureaucrat    (bureaucrat_action(silver)  , "Bureaucrat"    , 4, 0, 0, {ACTION,ATTACK});
const Card cellar        (cellar_action              , "Cellar"        , 2, 0, 0, {ACTION});
const Card chapel        (trash_action(4)            , "Chapel"        , 2, 0, 0, {ACTION});
const Card council_room  (council_room_action        , "Council Room"  , 5, 0, 0, {ACTION});
                       
const Card festival      (festival_action            , "Festival"      , 5, 0, 0, {ACTION});
const Card gardens       (gardens_action             , "Gardens"       , 4, 0, gardens_vp,
                                                                                  {VICTORY});
const Card laboratory    (laboratory_action          , "Laboratory"    , 5, 0, 0, {ACTION});
const Card library       (library_action             , "Library"       , 5, 0, 0, {ACTION});
const Card market        (market_action              , "Market"        , 5, 0, 0, {ACTION});
const Card militia       (militia_action             , "Militia"       , 4, 0, 0, {ACTION,ATTACK});
const Card mine          (mine_action                , "Mine"          , 5, 0, 0, {ACTION});
const Card moat          (moat_action                , "Moat"          , 2, 0, 0, {ACTION,REACTION});
const Card money_lender  (money_lender_action(copper), "Money Lender"  , 4, 0, 0, {ACTION});
const Card native_village(native_village_action      , "Native Village", 2, 0, 0, {ACTION});
const Card pirate_ship   (pirate_ship_action         , "Pirate Ship"   , 4, 0, 0, {ACTION,ATTACK});
const Card remodel       (remodel_action             , "Remodel"       , 4, 0, 0, {ACTION});
const Card smithy        (add_cards(3)               , "Smithy"        , 4, 0, 0, {ACTION});
const Card throne_room   (throne_room_action         , "Throne Room"   , 4, 0, 0, {ACTION});
const Card village       (village_action             , "Village"       , 3, 0, 0, {ACTION});
const Card witch         (witch_action(curse)        , "Witch"         , 5, 0, 0, {ACTION,ATTACK});
const Card workshop      (workshop_action            , "Workshop"      , 3, 0, 0, {ACTION});

const Card* const center_pile_cards[] = {
    &bureaucrat , &cellar      , &chapel      , &council_room, &festival,
    &gardens    , &laboratory  , &library     , &market      , &militia,
    &mine       , &moat        , &money_lender, &remodel     , &smithy, 
    &throne_room, &village     , &witch       , &workshop
};

////////////////////////////////////////////////////////////////////////////////
// Bot brains
////////////////////////////////////////////////////////////////////////////////

auto choose_random_action(Random* r) {
    return [=](const Game&                game,
               const Player&              player,
               const std::vector<Action>& actions)
    {
        assert(0 < actions.size());
        return actions[r->roll(actions.size())];
    };
}

auto choose_random_cards(Random* r) {
    return [=](const Game&                     game,
               const Player&                   player,
               const std::vector<const Card*>& cards,
               std::size_t                     at_least_,
               std::size_t                     at_most_)
    {
        const std::size_t at_most = std::min(at_most_, cards.size());
        assert(at_least_ <= at_most);
        if (at_most == 0 || cards.empty()) return cards;

        // Doing nothing should be rare
        if (at_least_ == 0 && r->roll(20) == 0)
            return std::vector<const Card*>{};
        const std::size_t at_least = std::max(at_least_, std::size_t(1));
        return r->deal(at_least + r->roll(at_most - at_least + 1), cards);
    };
}

auto big_spender(Random* r) {
    return [=](const Game&                     game,
               const Player&                   player,
               const std::vector<const Card*>& cards,
               std::size_t                     at_least,
               std::size_t                     at_most_)
    {
        const std::size_t at_most = std::min(at_most_, cards.size());
        assert(at_least <= at_most);
        if (at_most == 0 || cards.empty()) return cards;

        // If group changes so that keys are not ordered, the expression
        // to compute by_decreasing_cost must change as well.
        auto by_decreasing_cost = at(cards,
            reverse(value(group(each(L1(x->cost), cards)))));

        std::vector<const Card*> chosen;
        chosen.reserve(at_most);
        std::size_t n = 0;
        auto it = std::begin(by_decreasing_cost);
        for (; it != std::end(by_decreasing_cost) && n < at_most; ++it) {
            const std::size_t h = count(*it);
            append(chosen, n+h <= at_most? *it : r->deal(at_most - n, *it));
            n += h;
        }
        return chosen;
    };
}

////////////////////////////////////////////////////////////////////////////////
// Setup & game loop
////////////////////////////////////////////////////////////////////////////////

Deck init_deck(Random* r) {
    Deck deck(r);
    for (int i = 0; i < 7; ++i) deck.put_on_top(&copper);
    for (int i = 0; i < 3; ++i) deck.put_on_top(&estate);
    return deck;
}

struct PlayerInfo {
    std::string              name;
    Deck                     deck;
    Player::action_chooser_t action_chooser;
    Player::card_chooser_t   card_chooser;
    UI*                      ui;
};

std::map<const Card*, int>
init_piles(Random* r, std::size_t num_center_piles=10, int center_pile_depth=10) {
    assert(num_center_piles <= count(center_pile_cards));

    std::map<const Card*, int> piles{
        {&gold  , 30}, {&silver, 40}, {&copper  , 46}, {&curse, 10},
        {&estate,  8}, {&duchy ,  8}, {&province,  8}
    };
    for (const Card* c: r->deal(num_center_piles, center_pile_cards))
        piles[c] = center_pile_depth;
    return piles;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " player\n";
        return EXIT_FAILURE;
    }
    const char* const playername = argv[1];

    std::random_device rd;
    const auto seed = rd();
    std::cout << "Random seed: " << seed << '\n';
    Random randgen{std::mt19937_64{seed}};
    BotUI bui;
    CliUI cui(std::cin, std::cout);
    auto action_cli    = choose_action_cli   (std::cin, std::cout);
    auto card_cli      = choose_card_cli     (std::cin, std::cout);
    auto go_big        = big_spender         (&randgen);
    auto random_action = choose_random_action(&randgen);
    auto random_cards  = choose_random_cards (&randgen);
    Deck deck{init_deck(&randgen)};
    std::vector<Player> players{
        {playername   , deck, action_cli   , card_cli    , &cui},
        {"Mr Greedy"  , deck, random_action, go_big      , &bui},
        {"Lord Random", deck, random_action, random_cards, &bui}};
    Game g{each(L1(&x), players), init_piles(&randgen)};
    auto is_over = L0(players.end() != find_if(L1(x.victory_points() >= 20),
                                               players));

    for (auto& p: g.players) p->ui->begin_game(g);
    for (auto it = std::begin(g.players); !is_over(); ) {
        Player& p = **it;
        g.init_turn(p);
        for (auto& o: g.players) o->ui->begin_turn(g, *o);

        while (filter(L1(x->is(ACTION)), p.deck.hand).size() && g.turn.actions) {
            const Card* const action = p.ui->choose_action(g);
            if (!action) break;
            g.turn.played = action;
            // It is critical for some actions that the played card is
            // removed from the player's hand before the action happens.
            p.deck.discard(action);
            for (auto& o: g.players) o->ui->play(g, p, *action);
            (action->action).perform(g, p);
            p.ui->complete_action(g);
            --g.turn.actions;
        }

        // Other players may glean something from the playing of particular
        // treasure cards.  I punted on that here; we always play all of them.
        g.turn.phase = Turn::BUY;
        auto treasures = filter(L1(x->is(TREASURE)), p.deck.hand);
        for (auto& c: treasures) {
            g.turn.played = c;
            for (auto& o: g.players) o->ui->play(g, p, *c);
        }
        int coins = sum(each(L1(x->treasure_points), treasures));
        while (coins && g.turn.buys && g.affordable(coins).size()) {
            const Card* const bought = p.ui->choose_buy(g, coins);
            if (!bought) break;
            for (auto& o: g.players) o->ui->buy(g, p, *bought);
            p.deck.discard_pile.push_back(bought);
            g.remove_card_from_piles(bought);
            coins -= bought->cost;
            --g.turn.buys;
        }
        p.deck.end_turn();
        if (++it == std::end(g.players)) it = std::begin(g.players);
    }
    for (auto& p: g.players) p->ui->end_game(g);
    return EXIT_SUCCESS;
}
