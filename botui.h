#pragma once

////////////////////////////////////////////////////////////////////////////////
// Implements the UI interface for a bot, which does almost nothing.  The only
// methods that do anything are the choose* methods, which delegate to the
// corresponding members of the current Player.
////////////////////////////////////////////////////////////////////////////////

#include <ui.h>

struct BotUI: UI {
    void        begin_game     (const Game&)                              override;
    void        begin_turn     (const Game&, const Player&)               override;
    void        buy            (const Game&, const Player&, const Card& ) override;
    const Card* choose_action  (const Game&)                              override;
    void        complete_action(const Game&)                              override;
    const Card* choose_buy     (const Game&, int coins)                   override;
    void        end_game       (const Game&)                              override;
    void        gains          (const Game&, const Player&, const Card& ) override;
    void        no_more        (const Game&, const Player&, const Card& ) override;
    void        play           (const Game&, const Player&, const Card& ) override;
    void        react          (const Game&, const Player&, const Card& ) override;
    void        show_cards     (const Game&, const Player&, const Cards&) override;
    void        trash          (const Game&, const Player&, const Cards&) override;
};
