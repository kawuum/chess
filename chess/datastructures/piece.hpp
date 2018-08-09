//
// Created by Siegfried Depner on 7/12/18.
//

#ifndef PIECE_HPP
#define PIECE_HPP

#include "enums.hpp"

class piece {

 public:

  piece() :valid(false) {
  }
     
  piece(piece_type p_in, piece_color c_in) : valid(true), p(p_in), c(c_in) {
  }

  piece_type get_piece_type() {
    return p;
  }

  void set_piece_type(piece_type p_in) {
    p = p_in;
  }

  piece_color get_piece_color() {
    return c;
  }

  void set_piece_color(piece_color c_in) {
    c = c_in;
  }

  bool is_white() {
    return (c == piece_color::WHITE);
  }
  
  bool is_valid() {
    return valid;
  }

 private:
  bool valid;
  piece_type p;
  piece_color c;
};

#endif //CHESS_PIECE_HPP
