#include "../datastructures/piece.hpp"

#ifndef STATEINFO_HPP
#define STATEINFO_HPP
/*
enum move_type {
    MOVE,
    CAPTURE,
    ENPASSANT,
    CASTLING,
    PROMOTION
};*/

struct move {
  piece moved_piece;
  int prev_x;
  int prev_y;
  int new_x;
  int new_y;
  move_type type;
  piece captured_piece;
};

struct state_info {
  int num_halfmoves;
  piece_color color_to_move;
  move last_move;
  state_info *prev;
};

#endif
