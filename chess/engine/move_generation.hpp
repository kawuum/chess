#include "../datastructures/piece.hpp"
#include <stdint.h>
#include <vector>
#include "move.hpp"
#include "../datastructures/board.hpp"
#include "game_history.hpp"

#ifndef MOVEGENERATION_HPP
#define MOVEGENERATION_HPP

class move_generation {
 public:
  std::vector<move> generate_moves(piece mover, uint8_t from_x, uint8_t from_y, std::shared_ptr<game_history> gh);
  std::vector<move> generate_all_moves(game_history* gh);
  bool is_check(board &b, move m);
 private:
  int counter;
  std::vector<move> generate_moves(piece mover, uint8_t from_x, uint8_t from_y, board &b, bool check_for_check = false);
  std::vector<move> generate_all_moves(board &b, piece_color to_move, bool check_for_check = false);
  void bishopmoves(piece mover,
                   uint8_t from_x,
                   uint8_t from_y,
                   board &b,
                   std::vector<move> &moves,
                   bool check_for_check = false);
  void rookmoves(piece mover,
                 uint8_t from_x,
                 uint8_t from_y,
                 board &b,
                 std::vector<move> &moves,
                 bool check_for_check = false);
  void pawnmoves(piece mover,
                 uint8_t from_x,
                 uint8_t from_y,
                 board &b,
                 std::vector<move> &moves,
                 bool check_for_check = false);
  void knightmoves(piece mover,
                   uint8_t from_x,
                   uint8_t from_y,
                   board &b,
                   std::vector<move> &moves,
                   bool check_for_check = false);
  void kingmoves(piece mover,
                 uint8_t from_x,
                 uint8_t from_y,
                 board &b,
                 std::vector<move> &moves,
                 bool check_for_check = false);
  void queenmoves(piece mover,
                  uint8_t from_x,
                  uint8_t from_y,
                  board &b,
                  std::vector<move> &moves,
                  bool check_for_check = false);
  void bishopmoves_helper(piece &mover,
                          board &b,
                          bool *stopped,
                          uint8_t from_x,
                          uint8_t from_y,
                          int8_t x_diff,
                          int8_t y_diff,
                          std::vector<move> &moves,
                          bool check_for_check);
  void rookmoves_helper(piece &mover,
                        board &b,
                        bool *stopped,
                        uint8_t from_x,
                        uint8_t from_y,
                        int8_t x_diff,
                        int8_t y_diff,
                        std::vector<move> &moves,
                        bool check_for_check);
  game_history gamestate;
};

#endif
