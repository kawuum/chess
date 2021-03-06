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
  bool is_check;
  
  game_history() {
      printf("Constructor called...\n");
  }
  
  ~game_history() {
      printf("Destructor called...\n");
  }
  
  bool operator==(game_history &rhs) {
    // num_halfmoves, color to move and the board should be enough to determine whether we are in fact dealing with the same game
    return ((num_halfmoves == rhs.num_halfmoves) && (to_move == rhs.to_move) && (curr_board == rhs.curr_board));
  }
  
  bool operator!=(game_history &rhs) {
      return !((num_halfmoves == rhs.num_halfmoves) && (to_move == rhs.to_move) && (curr_board == rhs.curr_board));
  }
};


#endif
