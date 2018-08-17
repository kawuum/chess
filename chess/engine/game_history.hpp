#include "../datastructures/board.hpp"
#include "../datastructures/enums.hpp"
#include <memory>
#include "move.hpp"

#ifndef GAMEHISTORY_HPP
#define GAMEHISTORY_HPP

struct game_history {
  int num_halfmoves;
  board curr_board;
  std::shared_ptr<game_history> prev;
  piece_color to_move;
  castling_rights castlingrights[2];
  std::tuple<uint8_t, uint8_t> ep_square;
  move performed_move;
  game_result result;
};

#endif
