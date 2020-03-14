#pragma once

////////////////////////////////////////////////////////////////////////////////
// Implements the UI interface using iostreams for user interaction.  CliUI
// doesn't use the istream passed to its ctor at all, since user input is for
// making choices, which is handled by the two functions defined at the bottom
// of this file.  That the choose functions are separate bugs me.
////////////////////////////////////////////////////////////////////////////////

#include <card.h>
#include <iosfwd>
#include <player.h>
#include <ui.h>
#include <vector>

struct CliUI: UI {
    CliUI(std::istream& in, std::ostream& out);

    void        begin_game     (const Game&)                              override;
    void        begin_turn     (const Game&, const Player&)               override;
    void        buy            (const Game&, const Player&, const Card& ) override;
    const Card* choose_action  (const Game&)                              override;
    void        complete_action(const Game&)                              override;
    const Card* choose_buy     (const Game&, int coins)                   override;
    void        end_game       (const Game&)                              override;
    void        gains          (const Game&, const Player&, const Card& ) override;
    void        no_more        (const Game&, const Player&, const Card& ) override;
    void        notify   (const Game&, const Player&, const std::string&) override;
    void        play           (const Game&, const Player&, const Card& ) override;
    void        react          (const Game&, const Player&, const Card& ) override;
    void        show_cards     (const Game&, const Player&, const Cards&) override;
    void        trash          (const Game&, const Player&, const Cards&) override;

private:
    std::ostream& os;
};

Player::action_chooser_t choose_action_cli(std::istream&, std::ostream&);
Player::card_chooser_t   choose_card_cli  (std::istream&, std::ostream&);
