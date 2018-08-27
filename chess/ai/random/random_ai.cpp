#include "random_ai.hpp"
#include <cstdlib>

move random_ai::next_move()
{
    game_history gh = eng->get_current_gamestate();
    std::vector<move> moves = eng->get_all_legal_moves(&gh);
    return moves[rand() % moves.size()];
}
random_ai::~random_ai()
{
    // do something?
}



