#include "hokm/engine.hpp"
#include <algorithm>
#include <numeric>
#include <stdexcept>
namespace hokm {
namespace {
void seat(int p) {
    if (p < 0 || p > 3)
        throw std::invalid_argument("seat must be 0..3");
}
} // namespace
std::array<Card, 52> Engine::shuffled_deck() {
    std::array<Card, 52> d{};
    std::iota(d.begin(), d.end(), 0);
    std::shuffle(d.begin(), d.end(), rng_);
    return d;
}
void Engine::reset(std::uint64_t seed, int h) {
    if (h != -1)
        seat(h);
    rng_.seed(seed);
    state_ = State{};
    if (h == -1) {
        auto selection = shuffled_deck();
        int first = -1, partner = -1, p = 0;
        for (Card c : selection) {
            if (rank(c) == 12) {
                if (first == -1)
                    first = p;
                else {
                    partner = p;
                    break;
                }
            }
            do {
                p = (p + 1) % 4;
            } while (p == first);
        }
        // Rotate first ace to seat 0; swap partner into seat 2.
        for (int i = 0; i < 4; ++i)
            state_.initial_seating[i] = (first + i) % 4;
        auto it = std::find(state_.initial_seating.begin(), state_.initial_seating.end(), partner);
        std::iter_swap(state_.initial_seating.begin() + 2, it);
        h = 0;
    }
    state_.hakem = h;
    begin_round(shuffled_deck());
}
void Engine::give(int p, int n) {
    for (int i = 0; i < n; ++i)
        state_.hands[p] |= Hand{1} << state_.deck[state_.cursor++];
}
void Engine::begin_round(const std::array<Card, 52> &d) {
    auto score = state_.score;
    int h = state_.hakem;
    auto seating = state_.initial_seating;
    state_ = State{};
    state_.score = score;
    state_.hakem = h;
    state_.initial_seating = seating;
    state_.deck = d;
    state_.actor = h;
    state_.leader = h;
    give(h, 5);
}
void Engine::load_deal(const std::array<Card, 52> &d, int h, std::array<int, 2> score,
                       std::uint64_t seed) {
    seat(h);
    Hand seen = 0;
    for (Card c : d) {
        (void)suit(c);
        Hand bit = Hand{1} << c;
        if (seen & bit)
            throw std::invalid_argument("duplicate card in deal");
        seen |= bit;
    }
    for (int s : score)
        if (s < 0 || s >= 7)
            throw std::invalid_argument("initial match scores must be 0..6");
    // All validation precedes mutation.
    state_ = State{};
    rng_.seed(seed);
    state_.score = score;
    state_.hakem = h;
    begin_round(d);
}
std::array<bool, 56> Engine::legal_action_mask() const {
    std::array<bool, 56> m{};
    if (phase() == Phase::ChooseTrump) {
        for (int i = 52; i < 56; ++i)
            m[i] = true;
    } else if (phase() == Phase::Play) {
        Hand h = legal_cards(state_.hands[state_.actor],
                             state_.trick_size ? suit(state_.trick[0].card) : -1);
        for (int i = 0; i < 52; ++i)
            m[i] = (h & (Hand{1} << i)) != 0;
    }
    return m;
}
std::vector<int> Engine::legal_actions() const {
    auto m = legal_action_mask();
    std::vector<int> a;
    a.reserve(13);
    for (int i = 0; i < 56; ++i)
        if (m[i])
            a.push_back(i);
    return a;
}
void Engine::apply(const Action &a) {
    int id = encode(a);
    if (!legal_action_mask()[id])
        throw std::invalid_argument("illegal action for current phase, hand, or led suit");
    if (phase() == Phase::ChooseTrump) {
        state_.trump = id - 52;
        for (int i = 1; i < 4; ++i)
            give((state_.hakem + i) % 4, 5);
        for (int batch = 0; batch < 2; ++batch)
            for (int i = 0; i < 4; ++i)
                give((state_.hakem + i) % 4, 4);
        state_.phase = Phase::Play;
        return;
    }
    auto played = PlayedCard{state_.actor, id};
    state_.hands[state_.actor] &= ~(Hand{1} << id);
    state_.trick[state_.trick_size++] = played;
    state_.history[state_.history_size++] = played;
    if (state_.trick_size < 4) {
        state_.actor = (state_.actor + 1) % 4;
        return;
    }
    int winner = trick_winner(state_.trick, state_.trump);
    ++state_.tricks[winner % 2];
    state_.trick_size = 0;
    state_.trick = {};
    state_.leader = winner;
    state_.actor = winner;
    if (state_.tricks[winner % 2] == 7) {
        state_.result = score_round(state_.tricks, state_.hakem);
        state_.score[winner % 2] += state_.result.points;
        state_.actor = -1;
        state_.phase = Phase::RoundOver;
        if (state_.score[winner % 2] >= 7) {
            state_.phase = Phase::MatchOver;
            state_.match_winner = winner % 2;
        }
    }
}
void Engine::start_next_round() {
    if (phase() != Phase::RoundOver)
        throw std::invalid_argument("next round requires a completed nonterminal round");
    if (state_.result.winner != state_.hakem % 2)
        state_.hakem = (state_.hakem + 1) % 4;
    begin_round(shuffled_deck());
}
Observation Engine::observe(int p) const {
    seat(p);
    Observation o;
    o.player = p;
    o.team = p % 2;
    o.hakem = state_.hakem;
    o.dealer = (state_.hakem + 3) % 4;
    o.trump = state_.trump;
    o.actor = state_.actor;
    o.leader = state_.leader;
    o.phase = phase();
    o.hand = cards(state_.hands[p]);
    o.trick.assign(state_.trick.begin(), state_.trick.begin() + state_.trick_size);
    o.history.assign(state_.history.begin(), state_.history.begin() + state_.history_size);
    for (int i = 0; i < 4; ++i) {
        Hand h = state_.hands[i];
        while (h) {
            ++o.hand_sizes[i];
            h &= h - 1;
        }
    }
    o.tricks = state_.tricks;
    o.score = state_.score;
    o.result = state_.result;
    o.match_winner = state_.match_winner;
    if (p == state_.actor)
        o.legal_mask = legal_action_mask();
    return o;
}
void Engine::check_invariants() const {
    auto require = [](bool b) {
        if (!b)
            throw std::logic_error("engine invariant violated");
    };
    Hand seen = 0;
    auto add = [&](Card c) {
        require(c >= 0 && c < 52);
        Hand b = Hand{1} << c;
        require(!(seen & b));
        seen |= b;
    };
    for (Hand h : state_.hands) {
        require(!(h & ~full_deck));
        for (Card c : cards(h))
            add(c);
    }
    for (int i = state_.cursor; i < 52; ++i)
        add(state_.deck[i]);
    // History contains completed tricks AND current trick: count each card once.
    for (int i = 0; i < state_.history_size; ++i)
        add(state_.history[i].card);
    require(seen == full_deck);
    require(state_.history_size == 4 * (state_.tricks[0] + state_.tricks[1]) + state_.trick_size);
    for (int i = 0; i < state_.trick_size; ++i) {
        auto a = state_.trick[i], b = state_.history[state_.history_size - state_.trick_size + i];
        require(a.card == b.card && a.player == b.player);
        require(a.player == (state_.leader + i) % 4);
    }
    if (phase() == Phase::ChooseTrump)
        require(state_.cursor == 5 && state_.trump == -1 && state_.actor == state_.hakem);
    else
        require(state_.cursor == 52 && state_.trump >= 0 && state_.trump < 4);
    if (phase() == Phase::Play)
        require(state_.actor == (state_.leader + state_.trick_size) % 4 && state_.tricks[0] < 7 &&
                state_.tricks[1] < 7);
    if (phase() == Phase::RoundOver || phase() == Phase::MatchOver)
        require(state_.actor == -1 && state_.trick_size == 0 &&
                state_.result.tricks[state_.result.winner] == 7);
    require((phase() == Phase::MatchOver) == (state_.match_winner != -1));
}
} // namespace hokm
