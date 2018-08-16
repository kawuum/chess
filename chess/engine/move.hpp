#include "../datastructures/piece.hpp"
#include <stdint.h>

#ifndef MOVE_HPP
#define MOVE_HPP

enum move_type {
  MOVE,
  CAPTURE,
  ENPASSANT,
  CASTLING,
  PROMOTION,
  CAPTURING_PROMOTION
};

struct move {
  piece mover;
  uint8_t from_x;
  uint8_t from_y;
  uint8_t to_x;
  uint8_t to_y;
  move_type type;

  bool operator==(const move &other) {
    return this->from_x == other.from_x && this->from_y == other.from_y && this->to_x == other.to_x
        && this->to_y == other.to_y;
  }
};

#endif
