# include <iostream>
# include "classes.hpp"
# include "cli.cpp"
# include "init_game.cpp"
using namespace std;
int main()
{   
    GameState game = init_game();
    
    SetConsoleOutputCP(CP_UTF8);
    print_welcome_message();
    print_trash_cards(game);

    while (!game.is_finished())
    {   
        print_everything(game);

        int input = get_player_move(game);
        Card chosen = game.get_hand1()[input - 1];
        game.place_card(chosen, 1);

        print_separator();
        print_everything(game);

        // Simulate player 2 move: play first available card if exists we will replace with AI later on.
        if (!game.get_hand2().empty()) {
            chosen = game.get_hand2()[0]; 
            print_player2_move(chosen);
            game.place_card(chosen, 2);
            wait_for_continue();
        }

        game.gather_won_cards();
        print_separator();
    }
    print_game_result(game);
    return 0;
}