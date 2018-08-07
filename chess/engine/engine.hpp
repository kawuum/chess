#include "../datastructures/board.hpp"
#include "game_history.hpp"
#include "../datastructures/enums.hpp"
#include <memory>

#ifndef ENGINE_HPP
#define ENGINE_HPP

class engine
{
  public:
    engine() { }
    void new_game();
    void new_game(std::string fen);
    void perform_move(uint8_t from_x, uint8_t from_y, uint8_t to_x, uint8_t to_y);
    void perform_move(piece p, uint8_t from_x, uint8_t from_y, uint8_t to_x, uint8_t to_y);
    void undo_move();
    board& get_current_board();
  private:
    std::shared_ptr<game_history> gh;
};

#endif
