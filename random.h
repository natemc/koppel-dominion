#pragma once

////////////////////////////////////////////////////////////////////////////////
// Support for randomness:
//   deal   (n, x) -> n random elements from x without replacement
//   roll   (n)    -> uniform random in [0, n)
//   shuffle(x)    -> a shuffled copy of x
// Uses type erasure a la https://www.youtube.com/watch?v=bIhUE5uUFOA
////////////////////////////////////////////////////////////////////////////////

#include <algorithm>
#include <at.h>
#include <cassert>
#include <count.h>
#include <cstddef>
#include <iterator>
#include <memory>
#include <take.h>
#include <til.h>
#include <type_traits>
#include <utility>
#include <vector>

struct Random {
    template <class G>
    explicit Random(G g_): g(new model<G>(std::move(g_))) {}
    Random(const Random& x): g(x.g->clone()) {}
    Random(Random&&) noexcept = default;
    Random& operator=(Random x) { g = std::move(x.g); return *this; }
    Random& operator=(Random&&) noexcept = default;

    template <class C> auto deal(std::size_t n, const C& x) {
        return take(n, shuffle(x));
    }

    std::size_t roll(std::size_t n) {
        assert(0 < n);
        return g->roll(n);
    }

    template <class C> [[nodiscard]] auto shuffle(const C& c) {
        return at(c, g->shuffle(til(count(c))));
    }

private:
    using index_t = std::vector<std::size_t>;

    struct sig {
        virtual             ~sig   ()              = default;
        virtual sig*        clone  ()              = 0;
        virtual std::size_t roll   (std::size_t n) = 0; 
        virtual index_t     shuffle(index_t     x) = 0;
    };

    template <class G> struct model: sig {
        explicit model(G g_): g(std::move(g_)) {}

        model*      clone  ()              override { return new model(g); }
        std::size_t roll   (std::size_t n) override { return g() % n; }
        index_t     shuffle(index_t     x) override {
            std::shuffle(std::begin(x), std::end(x), g);
            return x;
        }

        G g;
    };

    std::unique_ptr<sig> g;
};
