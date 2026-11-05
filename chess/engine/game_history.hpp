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
//
// The order of the members saves memory: the board must start at a multiple
// of 8 bytes, so after the 4-byte num_halfmoves there are 4 bytes left, which
// the four 1-byte members fill. That keeps the struct at 200 bytes instead of
// 208 (see tests/memory_layout_test.cpp).
struct game_history {
  int num_halfmoves = 0;
  game_result result = RUNNING;
  draw_reason draw = NO_DRAW;  // set when result is DRAW
  bool is_check = false;
  // Half moves since the last capture or pawn move (50-move rule). One byte
  // is enough: at 100 the game is a draw.
  uint8_t halfmove_clock = 0;
  board curr_board;
  std::shared_ptr<game_history> prev;
  piece_color to_move = piece_color::WHITE;
  castling_rights castlingrights[2] = {NONE, NONE};
  // (0, 0) means "no en passant square"; move_generation relies on this.
  std::tuple<uint8_t, uint8_t> ep_square{0, 0};
  move performed_move{};

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
