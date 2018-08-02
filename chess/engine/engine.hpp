#include "../datastructures/board.hpp"
#include "game_history.hpp"
#include "../datastructures/enums.hpp"

#ifndef ENGINE_HPP
#define ENGINE_HPP

class engine
{
  public:
    engine() { }
    void new_game();
    void new_game(std::string fen);
    void perform_move(int from_x, int from_y, int to_x, int to_y);
    void perform_move(piece p, int from_x, int from_y, int to_x, int to_y);
    void undo_move();
    board& get_current_board();
  private:
    game_history* gh;
};

#endif
