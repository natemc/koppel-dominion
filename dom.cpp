#include <algorithm>
#include <any.h>
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
#include <event.h>
#include <except.h>
#include <filter.h>
#include <find.h>
#include <find_if.h>
#include <game.h>
#include <group.h>
#include <iostream>
#include <iterator>
#include <join.h>
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
    template <class C>
    const Card* choose_card_to_gain(Game& g, Player& p, C&& criteria) {
        auto gainable = filter(std::forward<C>(criteria),
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
        return Action{
            description,
            [=](Game& g, Player& p){
                assert(g.piles.count(card));
                if (!g.piles[card]) {
                    for (auto& o: g.players) o->ui->no_more(g, p, *card);
                }
                else {
                    --g.piles[card];
                    place(p, card);
                    for (auto& o: g.players) o->ui->gains(g, p, *card);
                }
            }
        };
    }

    template <class C, class P>
    Action gain_choice(const std::string& description, C criteria, P place) {
        return Action{
            description,
            [=](Game& g, Player& p) {
                if (auto chosen = choose_card_to_gain(g, p, criteria))
                    place(p, chosen);
            }};
    }

    // Returns the players who did not block (i.e., play Moat)
    std::vector<Player*> process_reactions(Game& g, Player& p) {
        std::vector<Player*> blockers;
        for (auto& o: filter(L1(x != &p), g.players)) {
            auto reactions = filter(L1(x->is(REACTION)), o->deck.hand);
            if (reactions.size()) {
                o->ui->notify(g, *o, "Choose reaction(s)");
                auto revealed = o->choose_cards(g, reactions);
                for (auto& r: revealed) {
                    for (auto q: g.players) q->ui->react(g, *o, *r);
                    r->perform_reaction(g, *o);
                }
                if (any(L1(x->is(BLOCK)), revealed)) blockers.push_back(o);
            }
        }
        return filter(L1(x != &p), g.players) ^except^ blockers;
    }

    // The usual pattern for attack cards is
    // 1) the player whose turn it is plays the attack card
    // 2) the other players react; some may nullify the attack
    // 3) the player who played the attack gets the gain effects
    // 4) other players who did not block suffer the attack effects
    Action gain_attack(Action gain, Action attack) {
        return Action{
            join("; ", {gain, attack}),
            [=](Game& g, Player& p, const Card* c) {
                const auto victims = process_reactions(g, p);
                gain(g, p, c);
                for (auto& o: victims) attack(g, *o, c);
            }
        };
    }

    void to_deck(Player& player, const Card* card) {
        player.deck.put_on_top(card);
    }

    void to_discard_pile(Player& player, const Card* card) {
        player.deck.discard_pile.push_back(card);
    }

    void to_hand(Player& player, const Card* card) {
        player.deck.hand.push_back(card);
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

const Action artisan_action = sequence_action({
    gain_choice("Gain a card to your hand costing up to 5 Coins",
                LNC1(x->cost <= 5), to_hand),
    {
        "Put a card from your hand onto your deck",
        [](Game& g, Player& p) {
            if (p.deck.hand.size()) {
                auto chosen = *std::begin(p.choose_exactly(g, p.deck.hand, 1));
                p.deck.hand.erase(find(p.deck.hand, chosen));
                p.deck.put_on_top(chosen);
            }
        }
    }
});

// It's not clear to me what the correct behavior is when one of the other
// players has only a single card in his or her draw pile.  In this
// implementation, the player in question only reveals that single card.
// Perhaps they must shuffle their discard pile, put the single card on
// top, and then show 2 cards.
Action bandit_action(const Card& gold, const Card& copper) {
    return gain_attack(
        gain_card(&gold, "Gain a gold", to_discard_pile),
        {
            "Each other player reveals the top 2 cards of their deck, trashes"
            " a revealed Treasure other than Copper, and discards the rest",
            [&](Game& g, Player& p) {
                const auto top2 = take(2, p.deck.draw_pile);
                for (auto& o: g.players) o->ui->show_cards(g, p, top2);
                const auto non_copper_treasure =
                    filter(L1(x != &copper && x->is(TREASURE)), top2);
                if (non_copper_treasure.empty())
                    p.deck.discard_from_draw_pile(top2);
                else {
                    auto chosen = *std::begin(non_copper_treasure.size() == 1? 
                        non_copper_treasure :
                        p.choose_exactly(g, non_copper_treasure, 1));
                    p.deck.trash_from_draw_pile(chosen);
                    for (auto& o: g.players) o->ui->trash(g, p, {chosen});
                    p.deck.discard_from_draw_pile(top2 ^except^ chosen);
                }
            }
        });
}

Action bureaucrat_action(const Card& silver) {
    return gain_attack(
        gain_card(&silver, "Gain a silver onto your deck", to_deck),
        {
            "Each other player reveals a Victory card from their hand and puts"
            " it onto their deck (or reveals a hand with no Victory cards)",
            [](Game& g, Player& p) {
                for (auto& o: filter(L1(x != &p), g.players)) {
                    const auto it = find_if(L1(x->is(VICTORY)), o->deck.hand);
                    if (it == o->deck.hand.end()) {
                        for (auto q: g.players)
                            q->ui->show_cards(g, *o, o->deck.hand);
                    }
                    else {
                        for (auto q: g.players) q->ui->show_cards(g, *o, {*it});
                        p.deck.put_on_top(*it);
                        p.deck.hand.erase(it);
                    }
                }
            }
        });
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

const Action council_room_action = sequence_action({
    add_cards(4),
    {"Each other player draws a card", [](Game& g, Player& p) {
        for (auto& o: filter(L1(x != &p), g.players)) o->deck.draw();
    }}
});

Action descriptive_action(const std::string& description) {
    return {description, [](Game&,Player&){}};
}

const Action festival_action = sequence_action({
    add_actions(2), add_buys(1), add_coins(2)
});

// TODO Now that we have a basic event system, rewrite the playing of cards
// to be the scheduling of imminent events.  Then effects can be accomplished
// by tinkering with the events stored in the event system.  For some effects
// (e.g., the enchantress effect), this will require distinguishing instances
// of the same card.  It's easy to create a game deck holding all the instances
// and then continue to use raw pointers as observing pointers.  We will need
// to fix all the places that currently check, e.g., whether a card is a Copper
// by comparing it to the one true copper.  Using the name is the obvious
// replacement.

bool g_nullify = false; // Hack to be replaced by making card actions into events

Event enchantress_event(const Card* card, Player& p, int turn) {
    return Event{
        card,
        "[Enchantress] the first time each other player plays an Action card"
        " on their turn, they get +1 Card and +1 Action instead of following"
        " its instructions",
        [&p,turn](const Game& g){ return g.turn.count > turn && g.turn.player == &p; },
        [turn,t=turn,action=static_cast<const Card*>(nullptr)]
            (const Game& g) mutable
        {
            if (t == turn) return false;
            if (t < g.turn.count) action = nullptr;
            t = g.turn.count;
            if (g.turn.phase != Turn::ACTION || !g.turn.played || action)
                return false;
            action = g.turn.played;
            return true;
        },
        [&p,card](Game& g) {
            // How to squash the played card's action?
            g.turn.player->deck.draw();
            ++g.turn.actions;
            g_nullify = true;
            g.schedule(enchantress_event(card, p, g.turn.count));
        }
    };
}

Event enchantress_completion_event(const Card* card, Player& p, int turn) {
    return Event{
        card,
        "[Enchantress] At the start of your next turn, +2 Cards",
        K(false),
        [&p,turn](const Game& g){ return g.turn.count > turn && g.turn.player == &p; },
        [&](Game& g) { p.deck.draw(2); }
    };
}

const Action enchantress_action = {
    "Until your next turn, the first time each other player plays an Action"
    " card on their turn, they get +1 Card and +1 Action instead of following"
    " its instructions.  At the start of your next turn, +2 Cards.",
    [](Game& g, Player& p, const Card* card) {
        g.schedule(enchantress_event(card, p, g.turn.count));
        g.schedule(enchantress_completion_event(card, p, g.turn.count));
    }
};

const Action gardens_action = descriptive_action(
    "Worth VP per 10 cards you have (round down)");

const Action harbinger_action = sequence_action({
    add_cards(1), add_actions(1), {
        "Look through your discard pile.  You may put a card from it onto"
        " your deck.",
        [](Game& g, Player& p) {
            auto& dis = p.deck.discard_pile;
            if (auto chosen = p.choose_card(g, dis)) {
                auto it = find(dis, chosen);
                assert(it != std::end(dis));
                dis.erase(it);
                p.deck.put_on_top(chosen);
            }
        }
    }
});

const Action horse_traders_action = sequence_action({
    add_buys(1), add_coins(3),
    {"Discard 2 cards", [](Game& g, Player& p) {
         if (std::size_t n = std::min(std::size_t{2}, p.deck.hand.size()))
             p.deck.discard(p.choose_exactly(g, p.deck.hand, n));
    }}
});

Event horse_traders_event(Player& p, const Card* card) {
    return Event{
        card,
        "[Horse Traders] At the start of your next turn, +1 Card and return"
        " your Horse Traders card to your hand",
        K(false),
        [&](const Game& g) { return g.turn.player == &p; },
        [&p,card](Game& g) {
            p.deck.draw(1);
            p.deck.aside.erase(find(p.deck.aside, card));
            p.deck.hand.push_back(card);
        }
    };
}

const Action horse_traders_reaction = {
    "When another player plays an Attack card, you may first set this aside"
    " from your hand.  If you do, then at the start of your next turn, +1 Card"
    " and return this to your hand",
    [](Game& g, Player& p, const Card* c) {
        p.deck.hand.erase(find(p.deck.hand, c));
        p.deck.aside.push_back(c);
        g.schedule(horse_traders_event(p, c));
    }
};

const Action laboratory_action = sequence_action({
    add_cards(1), add_actions(1)
});

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

const Action market_action = sequence_action({
    add_cards(1), add_actions(1), add_buys(1), add_coins(1)
});

Event merchant_event(const Card* merchant, int turn, const Card& silver) {
    std::ostringstream os;
    os << "[Merchant] The first time you play a Silver this turn ("
       << turn << "), +1 Coin";
    return Event{
        merchant,
        os.str(),
        [=](const Game& g){ return turn < g.turn.count; },
        [&](const Game& g){ return g.turn.played == &silver; },
        [ ](Game&       g){ ++g.turn.coins; }
    };    
}

Action merchant_action(const Card& silver) {
    return sequence_action({add_cards(1), add_actions(1), {
        "The first time you play a Silver this turn, +1 Coin",
        [&](Game& g, Player& p, const Card* c) {
            g.schedule(merchant_event(c, g.turn.count, silver));
        }}});
}

const Action militia_action = gain_attack(
    add_cards(2),
    {
        "Each other player discards down to 3 cards in hand",
        [](Game& g, Player& p) {
            Deck::Cards& hand = p.deck.hand;
            if (hand.size() > 3) {
                p.deck.discard(p.choose_exactly(g, hand, hand.size() - 3));
            }
        }
});

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
    "+2 Cards", [](Game& g, Player& p){ p.deck.draw(2); }
};

const Action moat_reaction = descriptive_action(
    "When another player plays an Attack card, you may first reveal this from"
    " your hand, to be unaffected by it.");

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

const Action native_village_action = sequence_action({
    add_cards(2),
    choice_action({native_village_top_card_to_mat,
                   native_village_mat_to_hand})
});

const Action nop_action{"", [](Game&,Player&){}};

const Action pirate_ship_add_coins = {
    "+1 Coin per Coin token on your Pirate Ship mat",
    [](Game& g, Player& p) {
        g.turn.coins += p.mats[Player::PIRATE_SHIP].coin_tokens;
    }
};

// The phrase, "trashes one of those Treasures that you choose," is unclear.
// The word "those" seems to refer to the revealed cards and so I implemented
// this so that the instruction to trash only applies when a revealed card is
// also a Treasure card.
Action pirate_ship_trash(std::vector<Player*> victims) {
    return Action{
        "Each other player reveals the top 2 cards of their deck, trashes one of"
        " those Treasures that you choose, and discards the rest, and then if"
        " anyone trashed a Treasure you add a Coin token to your Pirate Ship mat",
        [victims=std::move(victims)](Game& g, Player& p) {
            bool trash = false;
            for (auto& o: victims) {
                const auto top2 = take(2, o->deck.draw_pile);
                for (auto& q: g.players) q->ui->show_cards(g, *o, top2);
                auto treasures = filter(L1(x->is(TREASURE)), top2);
                if (treasures.empty())
                    o->deck.discard_from_draw_pile(top2);
                else {
                    trash = true;
                    auto chosen = p.choose_exactly(g, treasures, 1)[0];
                    o->deck.trash_from_draw_pile(chosen);
                    for (auto& q: g.players) q->ui->trash(g, *o, {chosen});
                    o->deck.discard_from_draw_pile(top2 ^except^ chosen);
                }
            }
            if (trash) ++p.mats[Player::PIRATE_SHIP].coin_tokens;
        }
    };
}

const Action pirate_ship_action = {
    "Choose one: +1 Coin per Coin token on your Pirate Ship mat; or each other"
    " player reveals the top 2 cards of their deck, trashes one of those"
    " Treasures that you choose, and discards the rest, and then if anyone"
    " trashed a Treasure you add a Coin token to your Pirate Ship mat",
    [](Game& g, Player& p, const Card* c) {
        auto victims = process_reactions(g, p);
        auto choice = choice_action({
            pirate_ship_add_coins, pirate_ship_trash(std::move(victims))
        });
        choice(g, p, c);
    }
};

const Action poacher_discard = {
    "Discard a card per empty Supply pile",
    [](Game& g, Player& p) {
        const auto how_many = std::min(p.deck.hand.size(),
                                       count(filter(L1(0 == x.second), g.piles)));
        p.deck.discard(p.choose_exactly(g, p.deck.hand, how_many));
    }
};

const Action poacher_action = sequence_action({
    add_cards(1), add_actions(1), add_coins(1), poacher_discard
});

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

const Action sentry_action = sequence_action({
    add_cards(1), add_actions(1), {
        "Look at the top 2 cards of your deck.  Trash and/or discard any"
        " number of them.  Put the rest back on top in any order.",
        [](Game& g, Player& p) {
            auto top2 = take(2, p.deck.draw_pile);
            p.ui->notify(g, p, "Which cards would you like to trash?");
            auto to_trash = p.choose_cards(g, top2);
            if (to_trash.size()) {
                p.deck.trash_from_draw_pile(to_trash);
                for (auto& o: g.players) o->ui->trash(g, p, to_trash);
                for (auto& c: to_trash) top2.erase(find(top2, c));
            }
            if (to_trash.size() < 2) {
                p.ui->notify(g, p, "Which cards would you like to discard?");
                auto to_discard = p.choose_cards(g, top2);
                if (to_discard.size()) {
                    p.deck.discard_from_draw_pile(to_discard);
                    for (auto& c: to_discard) top2.erase(find(top2, c));
                }
                if (top2.size() == 1) p.deck.put_on_top(*std::begin(top2));
                if (top2.size() == 2) {
                    p.ui->notify(g, p,
                                 "Which card do you want to put on top first?");
                    auto chosen = *std::begin(p.choose_exactly(g, top2, 1));
                    p.deck.put_on_top(chosen);
                    p.deck.put_on_top(*std::begin(filter(L1(x != chosen), top2)));
                }
            }
        }
    }
});

const Action throne_room_action = {
    "You may play an Action card from your hand twice",
    [](Game& g, Player& p) {
        auto actions = filter(L1(x->is(ACTION)), p.deck.hand);
        if (auto card = actions.empty()? nullptr : p.choose_card(g, actions)) {
            p.deck.play(card);
            for (auto& o: g.players) o->ui->play(g, p, *card);
            card->perform_action(g, p);
            for (auto& o: g.players) o->ui->play(g, p, *card);
            card->perform_action(g, p);
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

const Action vassal_action = {
    "Discard the top card of your deck.  If it's an Action card, you may play it.",
    [](Game& g, Player& p) {
        const Card* const card = p.deck.next();
        p.ui->show_cards(g, p, {card});
        if (!card->is(ACTION) || !p.choose_card(g, {card}))
            p.deck.discard_pile.push_back(card);
        else {
            p.deck.in_play.push_back(card);
            for (auto& o: g.players) o->ui->play(g, p, *card);
            card->perform_action(g, p);
        }
    }
};

const Action village_action = sequence_action({add_cards(1), add_actions(2)});

Action witch_action(const Card& curse) {
    return gain_attack(add_cards(2),
                       gain_card(&curse, "gains a Curse", to_discard_pile));
}

const Action workshop_action = gain_choice(
    "Gain a card costing up to 4 Coins", LNC1(x->cost <= 4), to_deck);

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

const Card artisan       (artisan_action             , "Artisan"       , 6, 0, 0, {ACTION});
const Card bandit        (bandit_action(gold, copper), "Bandit"        , 5, 0, 0, {ACTION,ATTACK});
const Card bureaucrat    (bureaucrat_action(silver)  , "Bureaucrat"    , 4, 0, 0, {ACTION,ATTACK});
const Card cellar        (cellar_action              , "Cellar"        , 2, 0, 0, {ACTION});
const Card chapel        (trash_action(4)            , "Chapel"        , 2, 0, 0, {ACTION});
const Card council_room  (council_room_action        , "Council Room"  , 5, 0, 0, {ACTION});
const Card enchantress   (enchantress_action         , "Enchantress"   , 3, 0, 0, {ACTION,ATTACK,DURATION});
const Card festival      (festival_action            , "Festival"      , 5, 0, 0, {ACTION});
const Card gardens       (gardens_action             , "Gardens"       , 4, 0, gardens_vp,
                                                                                  {VICTORY});
const Card harbinger     (harbinger_action           , "Harbinger"     , 3, 0, 0, {ACTION});
const Card horse_traders (horse_traders_action       , "Horse Traders" , 4, 0, 0, {ACTION,REACTION},
                          horse_traders_reaction);
const Card laboratory    (laboratory_action          , "Laboratory"    , 5, 0, 0, {ACTION});
const Card library       (library_action             , "Library"       , 5, 0, 0, {ACTION});
const Card market        (market_action              , "Market"        , 5, 0, 0, {ACTION});
const Card merchant      (merchant_action(silver)    , "Merchant"      , 3, 0, 0, {ACTION});
const Card militia       (militia_action             , "Militia"       , 4, 0, 0, {ACTION,ATTACK});
const Card mine          (mine_action                , "Mine"          , 5, 0, 0, {ACTION});
const Card moat          (moat_action                , "Moat"          , 2, 0, 0, {ACTION,REACTION,BLOCK},
                          moat_reaction);
const Card money_lender  (money_lender_action(copper), "Money Lender"  , 4, 0, 0, {ACTION});
const Card native_village(native_village_action      , "Native Village", 2, 0, 0, {ACTION});
const Card pirate_ship   (pirate_ship_action         , "Pirate Ship"   , 4, 0, 0, {ACTION,ATTACK});
const Card poacher       (poacher_action             , "Poacher"       , 4, 0, 0, {ACTION});
const Card remodel       (remodel_action             , "Remodel"       , 4, 0, 0, {ACTION});
const Card sentry        (sentry_action              , "Sentry"        , 5, 0, 0, {ACTION});
const Card smithy        (add_cards(3)               , "Smithy"        , 4, 0, 0, {ACTION});
const Card throne_room   (throne_room_action         , "Throne Room"   , 4, 0, 0, {ACTION});
const Card vassal        (vassal_action              , "Vassal"        , 3, 0, 0, {ACTION});
const Card village       (village_action             , "Village"       , 3, 0, 0, {ACTION});
const Card witch         (witch_action(curse)        , "Witch"         , 5, 0, 0, {ACTION,ATTACK});
const Card workshop      (workshop_action            , "Workshop"      , 3, 0, 0, {ACTION});

const Card* const center_pile_cards[] = {
    &artisan      , &bandit     , &bureaucrat, &cellar      , &chapel        ,
    &council_room , &enchantress, &festival  , &gardens     , &harbinger     ,
    &horse_traders, &laboratory , &library   , &market      , &merchant      ,
    &militia      , &mine       , &moat      , &money_lender, &native_village,
    &pirate_ship  , &poacher    , &remodel   , &sentry      , &smithy        ,
    &throne_room  , &vassal     , &village   , &witch       , &workshop
};

////////////////////////////////////////////////////////////////////////////////
// Bot brains
////////////////////////////////////////////////////////////////////////////////

auto choose_random_action(Random* r) {
    return [=](const Game&                game,
               const Player&              player,
               const std::vector<Action>& actions) -> const Action&
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

std::size_t random_seed() {
    std::random_device rd;
    return rd();
}

int usage(const char* prog) {
    std::cerr << "Usage: " << prog << " [-s seed] player\n";
    return EXIT_FAILURE;
}

int main(int argc, char* argv[]) {
    const char* playername = nullptr;
    std::size_t seed       = random_seed();
    for (int i = 1; i < argc; ++i) {
        const char* const arg = argv[i];
        if (arg[0] == '-') {
            if (i == argc) return usage(argv[0]);
            if (arg[1] != 's') return usage(argv[0]);
            std::istringstream is(argv[++i]);
            if (!(is >> seed)) return usage(argv[0]);
        }
        else if (!playername) playername = argv[i];
        else return usage(argv[0]);
    }
    if (!playername) return usage(argv[0]);

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
    Game g{randgen.shuffle(each(L1(&x), players)), init_piles(&randgen)};
    auto is_over = L0(players.end() != find_if(L1(x.victory_points() >= 20),
                                               players));

    for (auto& p: g.players) p->ui->begin_game(g);
    for (auto it = std::begin(g.players); !is_over(); ) {
        Player& p = **it;
        g.init_turn(p);
        for (auto& o: g.players) o->ui->begin_turn(g, *o);
        g.process_events();

        while (any(L1(x->is(ACTION)), p.deck.hand) && g.turn.actions) {
            const Card* const action = p.ui->choose_action(g);
            if (!action) break;
            g.turn.played = action;
            // It is critical for some actions that the played card is
            // removed from the player's hand before the action happens.
            p.deck.play(action);
            for (auto& o: g.players) o->ui->play(g, p, *action);
            g.process_events();
            if (!g_nullify) action->perform_action(g, p);
            g_nullify = false;
            p.ui->complete_action(g);
            --g.turn.actions;
        }

        // Other players may glean something from the playing of particular
        // treasure cards.  I punted on that here; we always play all of them.
        g.turn.phase = Turn::BUY;
        auto treasures = filter(L1(x->is(TREASURE)), p.deck.hand);
        for (auto& c: treasures) {
            g.turn.played = c;
            p.deck.play(c);
            for (auto& o: g.players) o->ui->play(g, p, *c);
            g.process_events();
            g.turn.coins += c->treasure_points;
        }
        while (g.turn.coins && g.turn.buys && g.affordable().size()) {
            const Card* const bought = p.ui->choose_buy(g, g.turn.coins);
            if (!bought) break;
            for (auto& o: g.players) o->ui->buy(g, p, *bought);
            g.process_events();
            p.deck.discard_pile.push_back(bought);
            g.remove_card_from_piles(bought);
            g.turn.coins -= bought->cost;
            --g.turn.buys;
        }
        g.process_events();
        p.deck.end_turn();
        if (++it == std::end(g.players)) it = std::begin(g.players);
    }
    for (auto& p: g.players) p->ui->end_game(g);
    return EXIT_SUCCESS;
}
