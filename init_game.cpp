#include "classes.hpp"
#include <algorithm>
static const char hokm = 'd';
static vector<Card> card52 = [](){
    vector<Card> tmp;
    for (int i = 1; i <= 13; i++) {
        tmp.push_back(Card(i, 'c'));
        tmp.push_back(Card(i, 'd'));
        tmp.push_back(Card(i, 'h'));
        tmp.push_back(Card(i, 's'));
    }
    random_shuffle(tmp.begin(), tmp.end());
    return tmp;}
();
int card52_index = 0;
vector<Card> random_hand() {
    vector<Card> tmp;
    for (int i = 0; i < 13; i++) {
        tmp.push_back(card52[card52_index++]);
    }
    return tmp;
}
array<Card, 13> random_trash() {
    array<Card, 13> tmp;
    for (int i = 0; i < 13; i++) {
        tmp[i] = card52[card52_index++];
    }
    return tmp;
}  

GameState init_game() {

    vector<Card> hand1 = random_hand(); sort(hand1.begin(), hand1.end(), compareForSort);
    vector<Card> hand2 = random_hand(); sort(hand2.begin(), hand2.end(), compareForSort);

    array<Card, 13> trash1 = random_trash(); sort(trash1.begin(), trash1.end(), compareForSort);
    array<Card, 13> trash2 = random_trash(); sort(trash2.begin(), trash2.end(), compareForSort);
    
    GameState game(hokm, hand1, hand2, trash1, trash2);
    return game;
}   




