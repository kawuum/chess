#ifndef PIECE_HPP
#define PIECE_HPP

#include "enums.hpp"

class piece {
 public:
  // An invalid piece ("no piece"), e.g. for an empty square.
  piece() = default;

  piece(piece_type p_in, piece_color c_in) : valid(true), p(p_in), c(c_in) {}

  piece_type get_piece_type() const { return p; }

  void set_piece_type(piece_type p_in) { p = p_in; }

  piece_color get_piece_color() const { return c; }

  void set_piece_color(piece_color c_in) { c = c_in; }

  bool is_white() const { return (c == piece_color::WHITE); }

  bool is_valid() const { return valid; }

 private:
  // Type and color of an invalid piece have no meaning, but they get fixed
  // values so that reading them is never undefined behavior.
  bool valid = false;
  piece_type p = KING;
  piece_color c = piece_color::WHITE;
};

#endif  // CHESS_PIECE_HPP
