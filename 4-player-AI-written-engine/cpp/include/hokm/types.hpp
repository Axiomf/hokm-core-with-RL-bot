#pragma once
#include <array>
#include <cstdint>
#include <variant>
#include <vector>
namespace hokm {
using Card = int; // suit * 13 + rank offset (2=0, A=12)
using Hand = std::uint64_t;
constexpr Hand full_deck = (Hand{1} << 52) - 1;
enum class Suit { Clubs, Diamonds, Hearts, Spades };
enum class Phase { ChooseTrump, Play, RoundOver, MatchOver };
struct ChooseTrump {
    Suit suit;
};
struct PlayCard {
    Card card;
};
using Action = std::variant<ChooseTrump, PlayCard>;
int encode(const Action &action);
Action decode(int id);
int suit(Card card);
int rank(Card card);
std::vector<Card> cards(Hand hand);
struct PlayedCard {
    int player = -1;
    Card card = -1;
};
struct RoundResult {
    int winner = -1;
    int points = 0;
    std::array<int, 2> tricks{};
    int hakem = -1;
};
struct Observation {
    int player = -1, team = -1, hakem = -1, dealer = -1, trump = -1, actor = -1, leader = -1;
    Phase phase = Phase::ChooseTrump;
    std::vector<Card> hand;
    std::vector<PlayedCard> trick, history;
    std::array<int, 4> hand_sizes{};
    std::array<int, 2> tricks{}, score{};
    RoundResult result;
    int match_winner = -1;
    std::array<bool, 56> legal_mask{}; // all false when observer is not actor
};
// Privileged value snapshot: never give this to a policy.
struct State {
    Phase phase = Phase::ChooseTrump;
    int hakem = 0, trump = -1, actor = 0, leader = 0, cursor = 0;
    std::array<Hand, 4> hands{};
    std::array<Card, 52> deck{};
    std::array<PlayedCard, 4> trick{};
    int trick_size = 0;
    std::array<PlayedCard, 52> history{};
    int history_size = 0;
    std::array<int, 2> tricks{}, score{};
    RoundResult result;
    int match_winner = -1;
    // canonical seat -> participant label before ace selection/reseating
    std::array<int, 4> initial_seating{0, 1, 2, 3};
};
} // namespace hokm
