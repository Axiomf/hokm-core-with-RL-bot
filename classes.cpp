# include "classes.hpp"
#include <algorithm>
using namespace std;

// Definitions for static members
static const char hokm;
array<Card, 13> GameState::trash1;
array<Card, 13> GameState::trash2;

Card::Card()
{
    rank = 0;
    suit = 'n';
}
Card::Card(int rank, char suit)
{
    this->rank = rank;
    this->suit = suit;
}
Card::~Card() {}

bool Card::operator==(const Card& other) const {
    return rank == other.rank && suit == other.suit;
}

GameState::GameState(char hokm, vector<Card> hand1, vector<Card> hand2, array<Card, 13> trash1, array<Card, 13> trash2)
{
    this->hokm = hokm;
    this->hand1 = hand1;
    this->hand2 = hand2;
    GameState::trash1 = trash1;
    GameState::trash2 = trash2;
    round = 1;
    turn = 0;         // initialize turn owner
    finished = false;  // initialize finished flag
    winner_player = 0; // initialize winner
}
GameState::~GameState() {}
void GameState:: place_card(Card card, int playernum) {
    if (playernum == 1) {
        played1 = card;
        hand1.erase(remove(hand1.begin(), hand1.end(), card), hand1.end());
    }
    else {
        played2 = card;
        hand2.erase(remove(hand2.begin(), hand2.end(), card), hand2.end());
    }
    turn = !turn;


}
int GameState::decide_round() const { 
    if (played1.get_suit() == played2.get_suit()) return played1.get_rank() < played2.get_rank();
    else if (played1.get_suit() == hokm) return 0;
    else if (played2.get_suit() == hokm) return 1;
    else return turn;   
}
void GameState::gather_won_cards() {
    int roundWinner = decide_round(); // 0: player1 wins, nonzero: player2 wins
    
    if (roundWinner == 0) { // player1 wins
        if (won1_index < 7)
            won1[won1_index++] = make_pair(played1, played2);
    }
    else { // player2 wins
        if (won2_index < 7)
            won2[won2_index++] = make_pair(played1, played2);
    }
    // Reset played cards to default values
    played1 = Card();
    played2 = Card();
    
    turn = roundWinner;
    if (won1_index == 7 || won2_index == 7) {
        finished = true;
        if (won1_index == 7)
            winner_player = 1;
        else
            winner_player = 2;
    }
    else
        round++;
}
bool GameState::check_win(){
    if (round < 7) return false;
    else if (won1_index == 7) return true;
    else if (won2_index == 7) return true;
    else return false;
}
void GameState::play_a_round() {
    
}
GameState::State GameState::get_state() const {
    // Build and return the state object with all private members.
    return State{
        hokm,
        round,
        turn,
        played1,
        played2,
        won1_index,
        won2_index,
        hand1,
        hand2,
        won1,
        won2,
        finished,
        winner_player
    };
}
bool compareForSort(const Card& a, const Card& b){// a is false, b is true
    if (a.get_suit() == b.get_suit()) return a.get_rank() < b.get_rank();
    else if (a.get_suit() == hokm) return false;
    else if (b.get_suit() == hokm) return true;
    else return a.get_suit() < b.get_suit();
}
vector<Card> GameState::get_legal_moves(int player) const{
    // it either has hokm or played, or it will play anything
    vector<Card> moves;

    vector<Card> clubs;
    vector<Card> diamonds;
    vector<Card> hearts;
    vector<Card> spades;

    bool first = played1.get_rank() == 0 && played2.get_rank() == 0;
    
    if (player == 0) {
        if (first) return hand1;
        for (const auto& card : hand1) { // assume it is second
            if (card.get_suit() == played1.get_suit()) moves.push_back(card);
            else if (card.get_suit() == hokm) moves.push_back(card); 
        }
        if (moves.size() == 0) return hand1;
        else return moves;
            
    }

    else {
        if (first) return hand2;
        for (const auto& card : hand2) { // assume it is second
            if (card.get_suit() == played2.get_suit()) moves.push_back(card);
            else if (card.get_suit() == hokm) moves.push_back(card); 
        }
        if (moves.size() == 0) return hand2;
        else return moves;
        
    }
    //sort(moves.begin(), moves.end(), compareForSort);
    return moves;
};