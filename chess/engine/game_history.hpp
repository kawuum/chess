#include <memory>
#include <tuple>

#include "../datastructures/board.hpp"
#include "../datastructures/enums.hpp"
#include "move.hpp"

#ifndef GAMEHISTORY_HPP
#define GAMEHISTORY_HPP

// Every member has a default value: engine::perform_move() creates a fresh
// game_history for each move and does not set every field itself. Before,
// result and ep_square stayed uninitialized, which caused a random
// "game over" and phantom en passant moves.
struct game_history {
  int num_halfmoves = 0;
  board curr_board;
  std::shared_ptr<game_history> prev;
  piece_color to_move = piece_color::WHITE;
  castling_rights castlingrights[2] = {NONE, NONE};
  // (0, 0) means "no en passant square"; move_generation relies on this.
  std::tuple<uint8_t, uint8_t> ep_square{0, 0};
  move performed_move{};
  game_result result = RUNNING;
  bool is_check = false;

  bool operator==(game_history& rhs) {
    // num_halfmoves, color to move and the board should be enough to determine
    // whether we are in fact dealing with the same game
    return ((num_halfmoves == rhs.num_halfmoves) && (to_move == rhs.to_move) &&
            (curr_board == rhs.curr_board));
  }

  bool operator!=(game_history& rhs) {
    return !((num_halfmoves == rhs.num_halfmoves) && (to_move == rhs.to_move) &&
             (curr_board == rhs.curr_board));
  }
};

#endif
