#include "hokm/engine.hpp"
#include <algorithm>
#include <iostream>
#include <numeric>
#include <stdexcept>
using namespace hokm;
#define CHECK(x)                                                                                   \
    do {                                                                                           \
        if (!(x))                                                                                  \
            throw std::runtime_error("check failed: " #x);                                         \
    } while (false)
template <class F> void rejects(F f) {
    bool caught = false;
    try {
        f();
    } catch (const std::invalid_argument &) {
        caught = true;
    }
    CHECK(caught);
}
std::array<Card, 52> ordered() {
    std::array<Card, 52> d;
    std::iota(d.begin(), d.end(), 0);
    return d;
}
// Arrange full 13-card hands into the documented 5-4-4 dealing order.
std::array<Card, 52> deck_for(std::array<std::array<Card, 13>, 4> hands, int h) {
    std::array<Card, 52> d{};
    std::array<int, 4> at{};
    int n = 0;
    for (int batch : {5, 4, 4})
        for (int i = 0; i < 4; ++i) {
            int p = (h + i) % 4;
            for (int j = 0; j < batch; ++j)
                d[n++] = hands[p][at[p]++];
        }
    return d;
}
std::array<Card, 52> suit_hands(int h = 0) {
    std::array<std::array<Card, 13>, 4> hands{};
    for (int p = 0; p < 4; ++p)
        for (int j = 0; j < 13; ++j)
            hands[p][j] = 13 * p + j;
    return deck_for(hands, h);
}
bool same_obs(const Observation &a, const Observation &b) {
    if (a.player != b.player || a.team != b.team || a.hakem != b.hakem || a.dealer != b.dealer ||
        a.trump != b.trump || a.actor != b.actor || a.leader != b.leader || a.phase != b.phase ||
        a.hand != b.hand || a.hand_sizes != b.hand_sizes || a.tricks != b.tricks ||
        a.score != b.score || a.legal_mask != b.legal_mask || a.match_winner != b.match_winner ||
        a.trick.size() != b.trick.size() || a.history.size() != b.history.size())
        return false;
    if (a.result.winner != b.result.winner || a.result.points != b.result.points ||
        a.result.tricks != b.result.tricks || a.result.hakem != b.result.hakem)
        return false;
    for (size_t i = 0; i < a.history.size(); ++i)
        if (a.history[i].player != b.history[i].player || a.history[i].card != b.history[i].card)
            return false;
    for (size_t i = 0; i < a.trick.size(); ++i)
        if (a.trick[i].player != b.trick[i].player || a.trick[i].card != b.trick[i].card)
            return false;
    return true;
}
void rules_tests() {
    for (int c = 0; c < 56; ++c)
        CHECK(encode(decode(c)) == c);
    rejects([] { decode(56); });
    rejects([] { encode(PlayCard{-1}); });
    Hand h = (Hand{1} << 0) | (Hand{1} << 13) | (Hand{1} << 51);
    CHECK(legal_cards(h, 0) == 1);
    CHECK(legal_cards(h, 2) == h); // optional trump
    CHECK(trick_winner({{{0, 0}, {1, 12}, {2, 25}, {3, 11}}}, 3) == 1);
    CHECK(trick_winner({{{0, 12}, {1, 39}, {2, 25}, {3, 40}}}, 3) == 3);
    CHECK(score_round({7, 1}, 0).points == 1);
    CHECK(score_round({7, 0}, 0).points == 2);
    CHECK(score_round({0, 7}, 0).points == 3);
    CHECK(score_round({7, 0}, 1).points == 3);
    rejects([] { score_round({7, 7}, 0); });
}
void dealing_and_isolation() {
    Engine e;
    auto d = ordered();
    e.load_deal(d, 2);
    CHECK(e.observe(2).hand == std::vector<Card>({0, 1, 2, 3, 4}));
    CHECK((e.observe(2).hand_sizes == std::array<int, 4>{0, 0, 5, 0}));
    CHECK(e.observe(2).dealer == 1);
    CHECK(e.current_actor() == 2);
    auto before = e.observe(2);
    rejects([&] { e.apply_id(0); });
    CHECK(same_obs(before, e.observe(2)));
    e.apply_id(54);
    CHECK((e.observe(2).hand_sizes == std::array<int, 4>{13, 13, 13, 13}));
    CHECK(e.debug_state().hands[2] == ((Hand{31}) | (Hand{15} << 20) | (Hand{15} << 36)));
    e.check_invariants();
    // Deck positions 5 and 10 belong to other players in both phases.
    Engine a, b;
    auto hidden = d;
    std::swap(hidden[5], hidden[10]);
    a.load_deal(d, 0);
    b.load_deal(hidden, 0);
    CHECK(same_obs(a.observe(0), b.observe(0)));
    CHECK(a.legal_action_mask() == b.legal_action_mask());
    a.apply_id(55);
    b.apply_id(55);
    CHECK(same_obs(a.observe(0), b.observe(0)));
    CHECK(a.legal_actions() == b.legal_actions());
    auto snapshot = a.observe(0);
    snapshot.hand.clear();
    CHECK(!a.observe(0).hand.empty());
    auto debug = a.debug_state();
    debug.hands[0] = 0;
    CHECK(a.debug_state().hands[0] != 0);
    auto invalid = d;
    invalid[0] = invalid[1];
    before = a.observe(0);
    rejects([&] { a.load_deal(invalid, 0); });
    CHECK(same_obs(before, a.observe(0)));
    rejects([&] { a.reset(0, 4); });
    CHECK(same_obs(before, a.observe(0)));
    rejects([&] { a.start_next_round(); });
    CHECK(same_obs(before, a.observe(0)));
}
void follow_suit_and_ordinary() {
    Engine e;
    e.load_deal(ordered(), 0);
    e.apply_id(55);
    e.apply_id(0);
    CHECK(e.current_actor() == 1);
    CHECK(e.legal_actions() == std::vector<int>({5, 6, 7, 8, 9}));
    auto before = e.observe(1);
    rejects([&] { e.apply_id(24); });
    CHECK(same_obs(before, e.observe(1)));
    std::array<std::array<Card, 13>, 4> hands{};
    for (int p = 0; p < 4; ++p)
        for (int j = 0; j < 13; ++j)
            hands[p][j] = 13 * p + j;
    std::swap(hands[0][12], hands[1][12]);
    e.load_deal(deck_for(hands, 0), 0);
    e.apply_id(52);
    for (int a : {0, 12, 26, 39})
        e.apply_id(a);
    CHECK(e.current_actor() == 1);
    CHECK(e.observe(0).tricks[1] == 1);
    while (e.phase() == Phase::Play)
        e.apply_id(e.legal_actions().front());
    CHECK(e.round_result().winner == 0);
    CHECK(e.round_result().points == 1);
    CHECK((e.round_result().tricks == std::array<int, 2>{7, 1}));
}
void ace_selection() {
    for (int seed = 0; seed < 100; ++seed) {
        std::mt19937_64 rng(seed);
        auto deck = ordered();
        std::shuffle(deck.begin(), deck.end(), rng);
        int first = -1, second = -1, p = 0;
        for (int c : deck) {
            if (c % 13 == 12) {
                if (first < 0)
                    first = p;
                else {
                    second = p;
                    break;
                }
            }
            p = (p + 1) % 4;
            if (p == first)
                p = (p + 1) % 4;
        }
        Engine e(seed);
        auto state = e.debug_state();
        CHECK(state.hakem == 0);
        CHECK(state.initial_seating[0] == first);
        CHECK(state.initial_seating[2] == second);
        deck = ordered();
        std::shuffle(deck.begin(), deck.end(), rng);
        CHECK(state.deck == deck); // full fresh deck after selection
    }
}
void lifecycle() {
    for (int trump : {0, 1}) {
        Engine e;
        e.load_deal(suit_hands(), 0, {0, 0});
        e.apply_id(52 + trump);
        for (int trick = 0; trick < 7; ++trick) {
            int leader = trick == 0 ? 0 : trump;
            CHECK(e.current_actor() == leader);
            for (int j = 0; j < 4; ++j) {
                CHECK(e.current_actor() == (leader + j) % 4);
                e.apply_id(e.legal_actions().front());
                e.check_invariants();
            }
            if (trick < 6)
                CHECK(e.phase() == Phase::Play);
        }
        CHECK(e.round_result().points == (trump == 0 ? 2 : 3));
        CHECK(e.phase() == Phase::RoundOver);
        CHECK(e.debug_state().history_size == 28);
        CHECK(e.observe(0).hand_sizes[0] == 6);
        auto clone = e.clone();
        rejects([&] { e.apply_id(1); });
        CHECK(same_obs(e.observe(0), clone.observe(0)));
        e.start_next_round();
        clone.start_next_round();
        CHECK(e.debug_state().deck == clone.debug_state().deck);
        CHECK(e.observe(0).hakem == trump);
    }
    Engine e;
    e.load_deal(suit_hands(), 0, {6, 0});
    e.apply_id(52);
    while (e.phase() == Phase::Play)
        e.apply_id(e.legal_actions().front());
    CHECK(e.phase() == Phase::MatchOver);
    CHECK(e.match_winner() == 0);
    CHECK(e.observe(0).score[0] == 8);
    CHECK(e.legal_actions().empty());
    rejects([&] { e.start_next_round(); });
    rejects([&] { e.apply_id(52); });
}
void randomized() {
    int ordinary = 0;
    for (int seed = 0; seed < 150; ++seed) {
        Engine e(seed);
        auto twin = e.clone();
        CHECK(e.debug_state().initial_seating[0] != e.debug_state().initial_seating[2]);
        auto seating = e.debug_state().initial_seating;
        std::sort(seating.begin(), seating.end());
        CHECK((seating == std::array<int, 4>{0, 1, 2, 3}));
        std::mt19937 driver(seed + 800);
        while (e.phase() != Phase::MatchOver) {
            e.check_invariants();
            if (e.phase() == Phase::RoundOver) {
                if (e.round_result().points == 1)
                    ++ordinary;
                int h = e.observe(0).hakem,
                    expected = e.round_result().winner == h % 2 ? h : (h + 1) % 4;
                e.start_next_round();
                twin.start_next_round();
                CHECK(e.observe(0).hakem == expected);
            } else {
                auto actions = e.legal_actions();
                int action = actions[driver() % actions.size()];
                auto mask = e.legal_action_mask();
                auto saved = e.clone();
                for (int i = 0; i < 56; ++i)
                    if (!mask[i]) {
                        rejects([&] { e.apply_id(i); });
                        break;
                    }
                CHECK(same_obs(e.observe(0), saved.observe(0)));
                e.apply_id(action);
                twin.apply_id(action);
                for (int p = 0; p < 4; ++p)
                    CHECK(same_obs(e.observe(p), twin.observe(p)));
            }
        }
        e.check_invariants();
    }
    CHECK(ordinary > 0);
    // Replay an explicit deal, independent of reset seed.
    Engine e(12), r;
    auto d = e.debug_state().deck;
    r.load_deal(d, e.debug_state().hakem);
    while (e.phase() == Phase::ChooseTrump || e.phase() == Phase::Play) {
        auto a = e.legal_actions().back();
        e.apply_id(a);
        r.apply_id(a);
        for (int p = 0; p < 4; ++p)
            CHECK(same_obs(e.observe(p), r.observe(p)));
    }
    auto c = e.clone();
    c.start_next_round();
    CHECK(e.phase() == Phase::RoundOver);
}
int main() {
    try {
        rules_tests();
        dealing_and_isolation();
        follow_suit_and_ordinary();
        ace_selection();
        lifecycle();
        randomized();
        std::cout << "All native checks passed (150 randomized matches).\n";
    } catch (const std::exception &e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
}
