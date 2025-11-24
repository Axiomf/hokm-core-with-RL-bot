#ifndef CLASSES_HPP
#define CLASSES_HPP

# include <vector>
# include <array>
using namespace std;
// A standard 52-card French-suited deck comprises 13 ranks in each of the four suits: clubs (♣), diamonds (♦), hearts (♥) and spades (♠). Each suit includes three court cards (face cards), King, Queen and Jack, with reversible (i.e. double headed) images. Each suit also includes ten numeral cards or pip cards, from one (Ace) to ten
// we work with 1 to 13 for the ranks and c , d, h, s for the suits
// "Ace" = 13, "King" = 12, "Queen" = 11, "Jack" = 10 and so on ... "2" = 1
class Card
{
private:
    int rank; 
    char suit;
public:
    Card();
    Card(int rank, char suit);
    bool operator==(const Card& other) const;

    int get_rank() const { return rank; }
    char get_suit() const { return suit; }

    ~Card();
};
class GameState
{
private:
    static array<Card, 13> trash1; // hidden!!!
    static array<Card, 13> trash2; 

    char hokm;
    int round;

    int turn;
    Card played1;
    Card played2;

    int won1_index = 0;
    int won2_index = 0;

    vector<Card> hand1;// hidden!!!
    vector<Card> hand2;
    
    array<pair<Card,Card>, 7> won1;
    array<pair<Card,Card>, 7> won2;

    bool finished;
    int winner_player;



public:
    GameState(char hokm,
        vector<Card> hand1, vector<Card> hand2,
        array<Card, 13> trash1, array<Card, 13> trash2 );
    ~GameState();
    
    // getters
    array<Card, 13> get_trash1() const { return trash1; }
    array<Card, 13> get_trash2() const { return trash2; }
    int get_round() const { return round; } 
    char get_hokm() const { return hokm; }
    int get_first() const { return turn; }
    Card get_played1() const { return played1; }
    Card get_played2() const { return played2; }
    vector<Card> get_hand1() const { return hand1; }
    vector<Card> get_hand2() const { return hand2; }
    array<pair<Card,Card>, 7> get_won1() const { return won1; }
    array<pair<Card,Card>, 7> get_won2() const { return won2; }
    bool is_finished() const { return finished; }
    int get_winner() const { return winner_player; }

    void place_card(Card card,int playernum);
    int decide_round() const;
    void gather_won_cards();
    bool check_win();
    void play_a_round();
    vector<Card> get_legal_moves(int player) const; // assumes the turns are handled correctly


    friend Card decide_bot_move(GameState& game);
    friend Card easy_bot(GameState& game);
        
    struct State {
        char hokm;
        int round;
        int turn;
        Card played1;
        Card played2;
        int won1_index;
        int won2_index;
        vector<Card> hand1;
        vector<Card> hand2;
        array<pair<Card,Card>, 7> won1;
        array<pair<Card,Card>, 7> won2;
        bool finished;
        int winner_player;
    };

    State get_state() const;

};
bool compareForSort(const Card& a, const Card& b);// a is false, b is true
#endif // CLASSES_HPP




