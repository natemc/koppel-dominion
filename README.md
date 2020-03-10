# koppel-dominion
Design exercise for Koppel's Dojo (Feb 2020 cohort)

## High-level design

The types representing the state of the game are the following records and enums:

```
Action
    description: string
    perform    : (Game, Player) -> Game

Card
    action         : Action
    cost           : int
    name           : string
    tags           : set<Tag>
    treasure_points: int
    victory_points : Deck -> int

Deck
    discard_pile   : [Card]
    draw_pile      : stack<Card>
    hand           : [Card]
    
Game
    players: [Player]
    piles  : map<Card, int>
    turn   : Turn
    
Mat
    cards      : [Card]
    coin_tokens: int

Player
    choose_action: (Game, [Action]) -> Action
    choose_cards : (Game, [Card], int, int) -> [Card]
    deck         : Deck
    mats         : map<NATIVE_VILLAGE | PIRATE_SHIP, Mat>
    name         : string
    ui           : UI

Tag = ACTION | TREASURE | VICTORY | ATTACK | REACTION | CURSE

Turn
    actions: int
    buys   : int
    coins  : int
    phase  : ACTION | BUY
    played : Card?
    player : Player
```

The C++ classes implementing the types described above are small (the largest, deck.cpp, is under 200 lines including whitespace and comments of both the .h and .cpp files) as they contain very little logic.  The fields are public, and the few methods in each class are for convenience.  Meanwhile, I cheat in two ways: 1) I use the addresses of objects (`Card`s and `Player`s, in particular) as their identity, and 2) I mutate, rather than regenerate, the game state (the players' decks, the piles, and the current turn).

In addition, there is a single object (i.e., service abstraction) type, `UI`.  Its primary function is to notify players of changes in the game state.  It also has two methods, choose action and choose_buy, that interact with the player by delegating to the `Player::choose_card` method.

```
UI
    begin_game     : Game -> unit
    begin_turn     : (Game, Player) -> unit
    buy            : (Game, Player, Card) -> unit
    choose_action  : Game -> Card
    choose_buy     : (Game, int) -> Card
    complete_action: Game -> unit
    end_game       : Game -> unit
    no_more        : (Game, Player, Card) -> unit
    play           : (Game, Player, Card) -> unit
    react          : (Game, Player, Card) -> unit
    show_cards     : (Game, Player, [Card]) -> unit
    trash          : (Game, Player, [Card]) -> unit
```

The `CliUI` component, which implements the `UI` interface and the menu to allow a human to make choices, is the second-largest component in the game and, by nature, the messiest part of the code.

Meanwhile, there are numerous support functions (e.g., `each`, `filter`, `group`, `over`) to help me code they way I like.

Lastly, the `Action` and `Card` instances and the game loop - all the rules of the game, really - are defined within the entry point file, dom.cpp.

## Features
The following cards (see http://wiki.dominionstrategy.com/index.php/List_of_cards for the full list in the real game) are implemented:

```
//         CARD           ACTION                       NAME           COST TP VP  TAGS
// ---------------------------------------------------------------------------------------------
const Card copper        (nop_action                 , "Copper"        , 0, 1, 0, {TREASURE});
const Card gold          (nop_action                 , "Gold"          , 6, 3, 0, {TREASURE});
const Card silver        (nop_action                 , "Silver"        , 3, 2, 0, {TREASURE});
                                                                                
const Card duchy         (nop_action                 , "Duchy"         , 5, 0, 3, {VICTORY});
const Card estate        (nop_action                 , "Estate"        , 2, 0, 1, {VICTORY});
const Card province      (nop_action                 , "Province"      , 8, 0, 6, {VICTORY});
                                                                                
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
```

This set includes the cards from the game's first Base edition (that were not subsequently removed in the second edition) plus `Native Village` and `Pirate Ship` from the Seaside edition.  Those last two were added at Jimmy's suggestion since he knew that they would require a lot of work :-).

A game pits a single human player against two bots (Mr Greedy, who's not that smart, and Lord Random, who is even less so).  The first player to reach 20 victory points wins.

## How actions are put together
An action is a description and a function.  One relatively simple action is `workshop_action`:
```
const Action workshop_action = {
    "Gain a card costing up to 4 Coins",
    [](Game& g, Player& p) {
        if (auto chosen = choose_card_to_gain(g, p, L1(x->cost <= 4)))
            p.deck.put_on_top(chosen);
    }
};
```
More complex actions can often be built out of other actions using the factory functions (declared in `action.h`) `sequence_action`, `choice_action`, and `do_to_others`:
```
const Action festival_action{sequence_action({
    add_actions(2), add_buys(1), add_coins(2)
})};

const Action pirate_ship_action{choice_action({
    pirate_ship_add_coins,
    sequence_action({pirate_ship_reveal, pirate_ship_trash})                                        
})};

Action witch_action(const Card& curse) {                                        
    return sequence_action({                                                    
        add_cards(2),                                                           
        do_to_others(gain_card(&curse, "gains a Curse", to_discard_pile))       
    });                                                                         
}
```

## How to build
I wrote this on an iMac running Mojave using Apple clang version 11.0.0.  The Makefile is gmake and it assumes clang++ with C++14 support.  I would expect it to work on Linux (changing clang++ to g++ if necessary); if you try it and it fails, let me know, and I will look into it.

```
$ make
..
clang++ -std=c++14 -g -O0 -ferror-limit=1 -o dom action.o append.o at.o botui.o card.o cliui.o concat.o count.o deck.o dom.o each.o filter.o find.o find_if.o game.o group.o join.o k.o key.o lambda.o mat.o max.o over.o player.o print_card.o random.o reverse.o sort.o sum.o tag.o take.o til.o turn.o ui.o value.o
$
```

## How to run
The executable, `dom`, takes a single command line argument which is required: the name of the human player.  Whenever input from the user is required, the game presents a menu and waits for input.  To illustrate, here is an abridged transcript from a game:

```
$ ./dom Me
Random seed: 4110684603
```
It prints the random seed used for this run.  The seed can be used to re-run the same match.
```
Piles: Copper T C0|T1|V0,Gold T C6|T3|V0,Silver T C3|T2|V0,Duchy V C5|T0|V5,Estate V C2|T0|V1,Province V C8|T0|V6,Curse C C0|T0|V-1,Chapel A C2|T0|V0,Council Room A C5|T0|V0,Gardens V C4|T0|V0,Laboratory A C5|T0|V0,Militia AK C4|T0|V0,Moat AR C2|T0|V0,Remodel A C4|T0|V0,Throne Room A C4|T0|V0,Witch AK C5|T0|V0,Workshop A C3|T0|V0
Me         : 7 Copper T C0|T1|V0, 3 Estate V C2|T0|V1
Mr Greedy  : 7 Copper T C0|T1|V0, 3 Estate V C2|T0|V1
Lord Random: 7 Copper T C0|T1|V0, 3 Estate V C2|T0|V1
```
Cards are shown with their tags followed by a few stats.  An `AK` tag set (`Militia`) means the card is `Action-Attack`.  `C6|T3|V0`, for example (`Gold`), means that this card costs 6 coins, is worth three treasure points and zero victory points.  (The victory point value for `Gardens` is 0 until it goes in a player's deck; however, the buy menu will show the card's current potential value.)

The Piles are selected randomly from the implemented cards (excluding treasure, victory, and curse cards); in each run, only a subset of cards will be available.

The order of the players is determined randomly.  This time we get to go first, and since we're playing the human, the game prints the contents of our hand:
```
Me's (3VP) turn: 4 Copper T C0|T1|V0, 1 Estate V C2|T0|V1
```
When there are no action cards in the current player's hand, the game goes straight to the buy phase.  In this phase, the game always plays all treasure cards from the player's hand - we don't have to tell it:
```
    Me plays Copper T C0|T1|V0
    Me plays Copper T C0|T1|V0
    Me plays Copper T C0|T1|V0
    Me plays Copper T C0|T1|V0
```
Once the treasure cards are played, the coins for the turn are known.  The game then presents a menu for us to choose what to buy.  It only shows cards that are available and that we can afford:
```
  Buys: 1  Coins: 4
    Select from 0 to 1 cards [p N for details; s to skip]:
        0|Copper T C0|T1|V0
        1|Silver T C3|T2|V0
        2|Estate V C2|T0|C1
        3|Curse C C0|T0|V0
        4|Chapel A C2|T0|V0
        5|Gardens V C4|T0|V1
        6|Militia AK C4|T0|V0
        7|Moat AR C2|T0|V0
        8|Remodel A C4|T0|V0
        9|Throne Room A C4|T0|V0
        10|Workshop A C3|T0|V0
    Selection: 8
    Me buys Remodel A C4|T0|V0
```
Now the bots get to go:
```
Mr Greedy's (3VP) turn
    Mr Greedy plays Copper T C0|T1|V0
    Mr Greedy plays Copper T C0|T1|V0
    Mr Greedy plays Copper T C0|T1|V0
    Mr Greedy plays Copper T C0|T1|V0
    Mr Greedy buys Throne Room A C4|T0|V0
Lord Random's (3VP) turn
    Lord Random plays Copper T C0|T1|V0
    Lord Random plays Copper T C0|T1|V0
    Lord Random plays Copper T C0|T1|V0
    Lord Random plays Copper T C0|T1|V0
    Lord Random buys Militia AK C4|T0|V0
```
Luckily on our next turn we end up with 5 coins.  That means we have lots of possible cards to to choose from.  Not sure which to buy?  The `p` command prints the action description for a card:
```
Me's (3VP) turn: 5 Copper T C0|T1|V0
    Me plays Copper T C0|T1|V0
    Me plays Copper T C0|T1|V0
    Me plays Copper T C0|T1|V0
    Me plays Copper T C0|T1|V0
    Me plays Copper T C0|T1|V0
  Buys: 1  Coins: 5
    Select from 0 to 1 cards [p N for details; s to skip]:
        0|Copper T C0|T1|V0
        1|Silver T C3|T2|V0
        2|Duchy V C5|T0|V3
        3|Estate V C2|T0|V1
        4|Curse C C0|T0|V0
        5|Chapel A C2|T0|V0
        6|Council Room A C5|T0|V0
        7|Gardens V C4|T0|V1
        8|Laboratory A C5|T0|V0
        9|Militia AK C4|T0|V0
        10|Moat AR C2|T0|V0
        11|Remodel A C4|T0|V0
        12|Throne Room A C4|T0|V0
        13|Witch AK C5|T0|V0
        14|Workshop A C3|T0|V0
    Selection: p 8
        Laboratory A C5|T0|V0 ; +1 Card; +1 Action
    Selection: p 13
        Witch AK C5|T0|V0 ; +2 Cards; Each other player gains a Curse
    Selection: 13
    Me buys Witch AK C5|T0|V0
Mr Greedy's (3VP) turn
    Mr Greedy plays Copper T C0|T1|V0
    Mr Greedy plays Copper T C0|T1|V0
    Mr Greedy plays Copper T C0|T1|V0
    Mr Greedy plays Copper T C0|T1|V0
    Mr Greedy plays Copper T C0|T1|V0
    Mr Greedy buys Council Room A C5|T0|V0
Lord Random's (3VP) turn
    Lord Random plays Copper T C0|T1|V0
    Lord Random plays Copper T C0|T1|V0
    Lord Random plays Copper T C0|T1|V0
    Lord Random buys Chapel A C2|T0|V0
```
This time we have an action card in our hand, so the game asks if we want to play it:
```
Me's (3VP) turn: 2 Copper T C0|T1|V0, 2 Estate V C2|T0|V1, 1 Remodel A C4|T0|V0
  Actions: 1
    Select from 0 to 1 cards [p N for details; s to skip]:
        0|Remodel A C4|T0|V0
    Selection: p 0
        Remodel A C4|T0|V0 Trash a card from your hand; gain a card costing up to 2 Coins more than it
    Selection: 0
    Me plays Remodel A C4|T0|V0: Trash a card from your hand; gain a card costing up to 2 Coins more than it
```
Playing the `Remodel` card leads to two more menus:
```
    Select from 0 to 1 cards [p N for details; s to skip]:
        0|Estate V C2|T0|V1
        1|Copper T C0|T1|V0
        2|Copper T C0|T1|V0
        3|Estate V C2|T0|V1
    Selection: 1
    Me trashed 1 Copper T C0|T1|V0
    Select from 0 to 1 cards [p N for details; s to skip]:
        0|Copper T C0|T1|V0
        1|Estate V C2|T0|V1
        2|Curse C C0|T0|V0
        3|Chapel A C2|T0|V0
        4|Moat AR C2|T0|V0
    Selection: 3
    Me gains a Chapel A C2|T0|V0
    New hand: 1 Copper T C0|T1|V0, 2 Estate V C2|T0|V1
```
Now we begin the buy phase for this turn.  Since we can't afford anything good, we skip:
```
    Me plays Copper T C0|T1|V0
  Buys: 1  Coins: 1
    Select from 0 to 1 cards [p N for details; s to skip]:
        0|Copper T C0|T1|V0
        1|Curse C C0|T0|V0
    Selection: s
    Skip...
Mr Greedy's (3VP) turn
    Mr Greedy plays Council Room A C5|T0|V0: ; +4 Cards; Each other player draws a card
    Mr Greedy plays Copper T C0|T1|V0
    Mr Greedy plays Copper T C0|T1|V0
    Mr Greedy plays Copper T C0|T1|V0
    Mr Greedy plays Copper T C0|T1|V0
    Mr Greedy plays Copper T C0|T1|V0
    Mr Greedy buys Witch AK C5|T0|V0
Lord Random's (3VP) turn
    Lord Random plays Militia AK C4|T0|V0: ; +2 Cards; Each other player discards down to 3 cards in hand
```
Lord Random forces us to dump 3 cards (thanks to Mr Greedy's `Council Room` play, we have six at the moment) from our hand, so we get another menu.  We specify the 3 cards to discard by separating the menu numbers with spaces:
```
    Select from 3 to 3 cards [p N for details; s to skip]:
        0|Remodel A C4|T0|V0
        1|Copper T C0|T1|V0
        2|Copper T C0|T1|V0
        3|Copper T C0|T1|V0
        4|Estate V C2|T0|V1
        5|Copper T C0|T1|V0
    Selection: 0 4 5
    Lord Random plays Copper T C0|T1|V0
..
```
A while later, we draw 3 action cards in our hand, and we select the `Witch`.  However, Lord Random blocks the `Curse` with his `Moat`:
```
...
Me's (9VP) turn: 1 Silver T C3|T2|V0, 1 Estate V C2|T0|V1, 1 Militia AK C4|T0|V0, 1 Moat AR C2|T0|V0, 1 Witch AK C5|T0|V0
  Actions: 1
    Select from 0 to 1 cards [p N for details; s to skip]:
        0|Witch AK C5|T0|V0
        1|Militia AK C4|T0|V0
        2|Moat AR C2|T0|V0
    Selection: 0
    Me plays Witch AK C5|T0|V0: ; +2 Cards; Each other player gains a Curse
    Mr Greedy gains a Curse C C0|T0|V-1
    Lord Random blocked attack with Moat AR C2|T0|V0
    New hand: 2 Copper T C0|T1|V0, 1 Silver T C3|T2|V0, 1 Estate V C2|T0|V1, 1 Militia AK C4|T0|V0, 1 Moat AR C2|T0|V0
```
We like the `Witch` card so much, we try to double-it up via the `Throne Room`, only to find we've already depleted the `Curse` cards:
```
Me's (4VP) turn: 1 Copper T C0|T1|V0, 1 Silver T C3|T2|V0, 1 Remodel A C4|T0|V0, 1 Throne Room A C4|T0|V0, 1 Witch AK C5|T0|V0
  Actions: 1
    Select from 0 to 1 cards [p N for details; s to skip]:
        0|Throne A Room C4|T0|V0
        1|Remodel A C4|T0|V0
        2|Witch A C5|T0|V0
    Selection: 0
    Me plays Throne Room A C4|T0|V0: You may play an Action card from your hand twice
    Select from 0 to 1 cards [p N for details; s to skip]:
        0|Remodel A C4|T0|V0
        1|Witch A C5|T0|V0
    Selection: 1
    Me plays Witch AK C5|T0|V0: ; +2 Cards; Each other player gains a Curse
    ! There are no more Curse C C0|T0|V-1 cards
    ! There are no more Curse C C0|T0|V-1 cards
    Me plays Witch AK C5|T0|V0: ; +2 Cards; Each other player gains a Curse
    ! There are no more Curse C C0|T0|V-1 cards
    ! There are no more Curse C C0|T0|V-1 cards
    New hand: 2 Copper T C0|T1|V0, 3 Silver T C3|T2|V0, 1 Militia AK C4|T0|V0, 1 Remodel A C4|T0|V0
..
```
And so it goes.
```
..
    Selection: 5
    Me buys Province V C8|T0|V6
Me, with 22 victory points, defeated
    Mr Greedy with 12 victory points,
and Lord Random with 3 victory points.
$
```
If you want to resign early, just press `Ctrl-D` to close stdin.

## Future Directions
### Testing
I'm confident I wrote a few bugs (several hundred LOC per day, what could go wrong?).  Though time-consuming, adding unit tests would not be difficult.  If I do decide to grow the implementation, it would be well worth the investment to put automated testing in place first.

### More Cards
With only seven more cards, I'd have a complete implementation of the game's Base edition as currently defined.  Some of those seven cards will require design changes.  Since those sorts of design issues are the point of the exercise, I'd like to look at these.

### Client-Server
This would make a fun challenge.  Maybe one of my friends who writes phone apps would write a mobile client.

### UI improvements
Obviously the current interface leaves a lot to be desired.  Currently, however, this direction doesn't interest me as much as the others.
