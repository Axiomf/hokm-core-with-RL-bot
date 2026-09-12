#include <iostream>
#include "classes.hpp" // added include for GameState
#include <string>
#include <windows.h>
// ...existing includes...
using namespace std;

string get_suit_char(char suit);
string get_rank_char(int rank);

void print_welcome_message() {
    cout << "--- SINGLE PLAYER HOKM ---" << endl;
}

void print_trash_cards(const GameState& game) {
    cout << "trash1: ";
    for (const auto& card : game.get_trash1()) {
        cout << get_suit_char(card.get_suit()) << get_rank_char(card.get_rank()) << " ";
    }
    cout << endl;
    cout << "trash2: ";
    for (const auto& card : game.get_trash2()) {
        cout << get_suit_char(card.get_suit()) << get_rank_char(card.get_rank()) << " ";
    }
    cout << endl << endl;
}

int get_player_move(const GameState& game) {
    int input;
    while (true) {
        cout << "Enter index of card to play 1 ... " << game.get_hand1().size() << ": ";
        cin >> input;
        if (cin.good() && input >= 0 && input < game.get_hand1().size()) {
            return input;
        }
        cout << "Invalid index!" << endl;
        cin.clear();
        cin.ignore(INT_MAX, '\n');
    }
}

void print_player2_move(const Card& card) {
    cout << "Player 2 played: " << get_suit_char(card.get_suit()) << get_rank_char(card.get_rank()) << " ";
}

void wait_for_continue() {
    int input;
    cout << "Enter 2 to continue...";
    cin >> input;
}

void print_separator() {
    cout << "-------------------------------------------------" << endl;
}

void print_game_result(const GameState& game) {
    cout << "Game finished!" << endl;
    cout << "Player " << game.get_winner() << " won!" << endl;
}

string get_suit_char(char suit) {
    switch (suit) {
        case 'c': return "♣";
        case 'd': return "♦";
        case 'h': return "♥";
        case 's': return "♠";
        default: return "?";
    }
}

string get_rank_char(int rank) {
    if (rank >= 1 && rank <= 9) {
        return to_string(rank + 1);
    }
    switch (rank) {
        case 10: return "J";
        case 11: return "Q";
        case 12: return "K";
        case 13: return "A";
        default: return "?";
    }
}

void print_everything(const GameState& game) {
    GameState::State state = game.get_state();

    cout << "ROUND " << state.round << " Hokm: " << get_suit_char(state.hokm)  << " Turn: " << state.turn + 1 << endl << endl;
    cout << "Player 1: " << endl;
    cout << "hand: ";
    for (const auto& card : state.hand1) {
        cout << get_suit_char(card.get_suit()) << get_rank_char(card.get_rank()) << " ";
    }
    cout << endl;
    cout << "played: " << get_suit_char(state.played1.get_suit()) << get_rank_char(state.played1.get_rank());  
    
    cout << " won: "; 
    for (int i = 0; i < state.won1_index; i++) { 
        cout << get_suit_char(state.won1[i].first.get_suit()) << get_rank_char(state.won1[i].first.get_rank()) << " " << get_suit_char(state.won1[i].second.get_suit()) << get_rank_char(state.won1[i].second.get_rank()) << " | "; 
    }
    cout << endl << endl;
    
    cout << "Player 2: " << endl;
    cout << "hand: ";
    for (const auto& card : state.hand2) {
        cout << get_suit_char(card.get_suit()) << get_rank_char(card.get_rank()) << " ";
    }
    cout << endl;
    cout << "played: " << get_suit_char(state.played2.get_suit()) << get_rank_char(state.played2.get_rank());  
    
    cout << " won: "; 
    for (int i = 0; i < state.won2_index; i++) { 
        cout << get_suit_char(state.won2[i].first.get_suit()) << get_rank_char(state.won2[i].first.get_rank()) << " " << get_suit_char(state.won2[i].second.get_suit()) << get_rank_char(state.won2[i].second.get_rank()) << " | "; 
    }
    cout << endl << endl;


}



