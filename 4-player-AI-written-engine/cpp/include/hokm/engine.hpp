#pragma once
#include "rules.hpp"
#include <random>
namespace hokm {
class Engine {
  public:
    explicit Engine(std::uint64_t seed = 0, int initial_hakem = -1) {
        reset(seed, initial_hakem);
    }
    void reset(std::uint64_t seed, int initial_hakem = -1);
    Phase phase() const {
        return state_.phase;
    }
    int current_actor() const {
        return state_.actor;
    }
    std::array<bool, 56> legal_action_mask() const;
    std::vector<int> legal_actions() const;
    void apply(const Action &action);
    void apply_id(int id) {
        apply(decode(id));
    }
    void start_next_round();
    Observation observe(int player) const;
    RoundResult round_result() const {
        return state_.result;
    }
    int match_winner() const {
        return state_.match_winner;
    }
    Engine clone() const {
        return *this;
    }
    State debug_state() const {
        return state_;
    }
    // Full ordered deck in dealing order; starts at trump selection, with 5 dealt.
    // Replaces match context; score must be nonterminal. RNG is independently seeded.
    void load_deal(const std::array<Card, 52> &deck, int hakem, std::array<int, 2> score = {0, 0},
                   std::uint64_t future_seed = 0);
    void check_invariants() const;

  private:
    State state_;
    std::mt19937_64 rng_;
    void begin_round(const std::array<Card, 52> &deck);
    void give(int player, int count);
    std::array<Card, 52> shuffled_deck();
};
} // namespace hokm
