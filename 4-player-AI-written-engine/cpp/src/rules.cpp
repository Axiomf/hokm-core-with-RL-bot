#include "hokm/rules.hpp"
#include <stdexcept>
namespace hokm {
int suit(Card c) {
    if (c < 0 || c >= 52)
        throw std::invalid_argument("card must be 0..51");
    return c / 13;
}
int rank(Card c) {
    (void)suit(c);
    return c % 13;
}
int encode(const Action &a) {
    if (auto p = std::get_if<PlayCard>(&a)) {
        (void)suit(p->card);
        return p->card;
    }
    int s = static_cast<int>(std::get<ChooseTrump>(a).suit);
    if (s < 0 || s > 3)
        throw std::invalid_argument("trump suit must be 0..3");
    return 52 + s;
}
Action decode(int id) {
    if (id < 0 || id >= 56)
        throw std::invalid_argument("action id must be 0..55");
    if (id < 52)
        return PlayCard{id};
    return ChooseTrump{static_cast<Suit>(id - 52)};
}
std::vector<Card> cards(Hand h) {
    std::vector<Card> out;
    out.reserve(13);
    for (int c = 0; c < 52; ++c)
        if (h & (Hand{1} << c))
            out.push_back(c);
    return out;
}
Hand legal_cards(Hand h, int led) {
    if (led < -1 || led > 3)
        throw std::invalid_argument("led suit must be -1..3");
    if (led < 0)
        return h;
    Hand following = h & (Hand{8191} << (13 * led));
    return following ? following : h;
}
int trick_winner(const std::array<PlayedCard, 4> &t, int trump) {
    if (trump < 0 || trump > 3)
        throw std::invalid_argument("trump must be 0..3");
    int best = 0;
    for (int i = 0; i < 4; ++i) {
        int s = suit(t[i].card), b = suit(t[best].card);
        if ((s == trump && b != trump) || (s == b && rank(t[i].card) > rank(t[best].card)))
            best = i;
    }
    return t[best].player;
}
RoundResult score_round(std::array<int, 2> t, int h) {
    if (h < 0 || h > 3 || t[0] < 0 || t[1] < 0 ||
        !((t[0] == 7 && t[1] < 7) || (t[1] == 7 && t[0] < 7)))
        throw std::invalid_argument("round score requires one team at seven tricks");
    int w = t[0] == 7 ? 0 : 1;
    return {w, t[1 - w] == 0 ? (w == h % 2 ? 2 : 3) : 1, t, h};
}
} // namespace hokm
