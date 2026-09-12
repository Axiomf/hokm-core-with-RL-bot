#pragma once
#include "types.hpp"
namespace hokm {
Hand legal_cards(Hand hand, int led_suit);
int trick_winner(const std::array<PlayedCard, 4> &trick, int trump);
RoundResult score_round(std::array<int, 2> tricks, int hakem);
} // namespace hokm
