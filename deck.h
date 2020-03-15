#pragma once

////////////////////////////////////////////////////////////////////////////////
// A Deck is a set of cards divided into 3 partitions:
//   a hand
//   a draw pile (which is a stack)
//   a discard pile
////////////////////////////////////////////////////////////////////////////////

#include <card.h>
#include <iosfwd>
#include <random.h>
#include <vector>

struct Deck {
    using Cards = std::vector<const Card*>;

    explicit Deck(Random* r);

    void        discard               (const Card* ); // hand -> discard
    void        discard               (const Cards&); // hand -> discard
    void        discard_from_draw_pile(const Card* ); // draw -> discard
    void        discard_from_draw_pile(const Cards&); // draw -> discard
    void        draw                  (int n=1);      // draw -> hand
    void        end_turn              ();             // all  -> draw & draw 5
    const Card* next                  ();             // draw -> return
    void        play                  (const Card*);  // hand -> play
    void        put_on_top            (const Card*);  // arg  -> draw
    void        put_on_top            (const Cards&); // arg  -> draw
    void        trash                 (const Card*);  // hand -> gone
    void        trash                 (const Cards&); // hand -> gone
    void        trash_from_draw_pile  (const Card*);  // draw -> gone
    void        trash_from_draw_pile  (const Cards&); // draw -> gone

    std::size_t size          () const;
    int         victory_points() const;
    Cards       whole         () const;

    Cards aside;
    Cards discard_pile;
    Cards draw_pile;
    Cards hand;
    Cards in_play;

private:
    Random* gen;

    void discard_from(Cards&, const Card*);
    void discard_from(Cards&, const Cards&);
    void trash_from  (Cards&, const Card*);
    void trash_from  (Cards&, const Cards&);
};

std::ostream& operator<<(std::ostream& os, const Deck& deck);
