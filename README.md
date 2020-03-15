# koppel-dominion
Design exercise for Koppel's Dojo (Feb 2020 cohort)

## High-level design

The types representing the state of the game are the following records and enums:

```
Action
    description: string
    perform    : (Game, Player, Card) -> Game

Card
    action         : Action
    cost           : int
    name           : string
    reaction       : Action
    tags           : set<Tag>
    treasure_points: int
    victory_points : Deck -> int

Deck
    aside          : [Card]
    discard_pile   : [Card]
    draw_pile      : stack<Card>
    hand           : [Card]

Event
    card       : Card
    description: string
    expire     : Game -> bool
    trigger    : Game -> bool
    fire       : Game -> Game

Game
    events : [Event]
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

Tag = ACTION | TREASURE | VICTORY | ATTACK | REACTION | CURSE | BLOCK | DURATION

Turn
    actions: int
    buys   : int
    coins  : int
    count  : int
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
    gains          : (Game, Player, Card) -> unit
    no_more        : (Game, Player, Card) -> unit
    notify         : (Game, Player, string) -> unit
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
//         CARD           ACTION (REACTION)            NAME           COST TP VP  TAGS
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
```

This set includes
* The Base set (2nd edition)
* `Native Village` and `Pirate Ship` (Seaside expansion).  These were added at Jimmy's suggestion to force the design to incorporate more complex state management.
* `Enchantress` (Empires expansion).  This card requires very complex event handling; the current implementation is a complete hack and suggests that it may be worth thinking of the game as a whole as a CEP system.
* `Horse Traders` (Cornucopia expansion).  This card requires event handling and it also has a complex reaction.

A game pits a single human player against two bots (Mr Greedy, who's not that smart, and Lord Random, who is even less so).  The first player to reach 20 victory points wins.

## How actions are put together
An action is a description and a function.  One relatively simple action is `moat_action`:
```
const Action moat_action = {                                                    
    "+2 Cards", [](Game& g, Player& p){ p.deck.draw(2); }                       
};
```
More complex actions can often be built out of other actions using the factory functions (declared in `action.h`) `sequence_action` and `choice_action`:
```
const Action festival_action{sequence_action({
    add_actions(2), add_buys(1), add_coins(2)
})};

const Action native_village_action = sequence_action({                          
    add_cards(2),                                                               
    choice_action({native_village_top_card_to_mat,                              
                   native_village_mat_to_hand})                                 
});
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
The executable, `dom`, takes a single required command line argument: the name of the human player.  You may also pass an optional random seed argument via `-s seed` in order to replay a particular game.

What the human player sees is printed to the terminal; the most important items are highlighted in various colors (not shown here).

Whenever input from the user is required, the game presents a menu and waits for input.  To illustrate, here is an abridged transcript from a game:

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

### The State Debate
To support the `Native Village` and `Pirate Ship` cards, I added mats to the `Player` class.  The rules for those (and related) cards consistently refer to *your* mat, i.e., the mat belonging to the player playing the card.  To me, it seemed natural and convenient to make `mats` a member of `Player`.

Some folks, including Jimmy, felt this was polluting the `Player` class.  After all, most dominion games don't include these mats.  The mats are a place to store state related to a certain constellation of cards that are part of specific expansion packs, but my `Player` class carries around a vestigial collection of mats no matter what.

Considering this perspective for a few days, I believe this is a valid complaint.  If we were trying to design a generic card-game framework, its `Player` class wouldn't have a notion of mats.  Besides, the original concept of a card game player as just some cards and a brain is very appealing.

It isn't clear, however, what the correct alternative design is.  Here are a few possibilities:

1. We introduce an inheritance hierarchy below `Player`.  Different subclasses are instantiated depending on the rule set / expansion packs being used in a particular match.  Each subclass may add data members for any state needed for that set of rules.  This leads to downcasting somewhere - or a template-based approach in which every type is a family of types and every possible combination is stamped out at compile time.
2. The `Player` class has some loose manner (e.g., map<string, any>) to store state.  Some convention would be established for code to work with this state.  This may not be different enough from the current solution to satisfy the mat haters, and I think it would be more error prone than the current solution.
3. The actions for the relevant cards close over the state they share.  This approach leaves the problem of how to present this state to the user.  In dominion, a player may inspect the contents of his or her mats at any time (but not the contents of other players' mats).  Making state stored in actions available to the UI would be messy.

So far, I haven't thought of a cure that isn't worse than the disease.

### Events
My initial implementation of the `Merchant` card did not behave as specified by the rules of the game, which state, "The first time you play a Silver this turn, +1 Coin."  Implementing this correctly meant that the game needed to trigger actions during the buy phase, which my original implementation never did.  Moreover, this action (+1 Coin), though triggered by the playing of a card, is triggered by the playing of a card *other than the one requiring the action to take place*.  My first implementation simply checked whether the player of the `Merchant` card had a `Silver` in his or her hand and, if one was found, added a coin immediately.  Though usually correct, there are scenarios in which this is wrong.

This challenge is not limited to the `Merchant` card; some expansion packs include other cards (such as http://wiki.dominionstrategy.com/index.php/Horse_Traders and http://wiki.dominionstrategy.com/index.php/Enchantress) that have effects after the current player's turn ends.  This suggested a need for a general way to schedule behavior in the future.  I adding this feature to the game and implementing all three cards (`Merchant`, `Horse Traders`, and `Enchantress`) wasn't too bad.

However, my current `Enchantress` implementation is awful: it uses card-specific, global mutable state that the game's main loop knows about.  The `Enchantress` card shows that, in order to make all manner of things possible, a card may need to do more then just schedule events in the future.  We need to ability to remove and modify events that are scheduled but have not yet occurred.  This means that playing a card, instead of invoking an action or adding coins to the current players turn, should schedule events that we expect to occur immediately.

### Client-Server
This would make a fun challenge.  Maybe one of my friends who writes phone apps would write a mobile client.

### UI improvements
Obviously the current interface leaves a lot to be desired.  Currently, however, this direction doesn't interest me as much as the others.
