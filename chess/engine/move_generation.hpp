#include <stdint.h>

#include <vector>

#include "../datastructures/board.hpp"
#include "../datastructures/piece.hpp"
#include "game_history.hpp"
#include "move.hpp"

#ifndef MOVEGENERATION_HPP
#define MOVEGENERATION_HPP

class move_generation {
 public:
  /**
   * All legal moves of the side to move in the given position. Also sets
   * gh->is_check and, if there are no legal moves, gh->result (checkmate or
   * stalemate).
   */
  std::vector<move> generate_all_moves(game_history* gh);
  bool is_check(board& b, move m);
  /**
   * True if a piece of color "by" attacks square (x, y) on board b.
   */
  static bool is_square_attacked(board& b, int x, int y, piece_color by);

 private:
  // All of these append the legal moves they find to "moves".
  void generate_all_moves(board& b, piece_color to_move,
                          std::vector<move>& moves);
  void generate_moves(piece mover, uint8_t from_x, uint8_t from_y, board& b,
                      std::vector<move>& moves);
  void bishopmoves(piece mover, uint8_t from_x, uint8_t from_y, board& b,
                   std::vector<move>& moves);
  void rookmoves(piece mover, uint8_t from_x, uint8_t from_y, board& b,
                 std::vector<move>& moves);
  void pawnmoves(piece mover, uint8_t from_x, uint8_t from_y, board& b,
                 std::vector<move>& moves);
  void knightmoves(piece mover, uint8_t from_x, uint8_t from_y, board& b,
                   std::vector<move>& moves);
  void kingmoves(piece mover, uint8_t from_x, uint8_t from_y, board& b,
                 std::vector<move>& moves);
  void queenmoves(piece mover, uint8_t from_x, uint8_t from_y, board& b,
                  std::vector<move>& moves);
  void bishopmoves_helper(piece& mover, board& b, bool* stopped, uint8_t from_x,
                          uint8_t from_y, int8_t x_diff, int8_t y_diff,
                          std::vector<move>& moves);
  void rookmoves_helper(piece& mover, board& b, bool* stopped, uint8_t from_x,
                        uint8_t from_y, int8_t x_diff, int8_t y_diff,
                        std::vector<move>& moves);
  // The position moves are generated for, set by generate_all_moves(). Only
  // looked at (castling rights, en passant square), never copied: it lives in
  // the caller for the whole call.
  const game_history* gamestate = nullptr;
};

#endif
