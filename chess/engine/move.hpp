#include <stdint.h>

#include <string>

#include "../datastructures/piece.hpp"

#ifndef MOVE_HPP
#define MOVE_HPP

enum move_type : uint8_t {
  NO_MOVE,
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

  bool operator==(const move& other) {
    // since we are (usually) only comparing moves from the same ply, the
    // coordinates should be enough to determine whether two moves are equal
    return this->from_x == other.from_x && this->from_y == other.from_y &&
           this->to_x == other.to_x && this->to_y == other.to_y;
  }

  bool operator!=(const move& other) {
    // since we are (usually) only comparing moves from the same ply, the
    // coordinates should be enough to determine whether two moves are equal
    return !(this->from_x == other.from_x && this->from_y == other.from_y &&
             this->to_x == other.to_x && this->to_y == other.to_y);
  }
};

/** True for moves that need a promotion piece (with or without capture). */
inline bool is_promotion(const move& m) {
  return m.type == PROMOTION || m.type == CAPTURING_PROMOTION;
}

/**
 * Formats a move in UCI notation: start and target square, e.g. "e2e4",
 * plus the promotion piece for promotions ("e7e8q").
 */
inline std::string to_uci(const move& m, piece_type promotion = QUEEN) {
  std::string s;
  s += static_cast<char>('a' + m.from_x);
  s += static_cast<char>('1' + m.from_y);
  s += static_cast<char>('a' + m.to_x);
  s += static_cast<char>('1' + m.to_y);
  if (is_promotion(m)) s += "kqbnrp"[promotion];  // same order as piece_type
  return s;
}

#endif
