#include <stdint.h>

#include <string>
#include <type_traits>

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
  // The piece a pawn promotes to: queen, rook, bishop or knight. Only
  // meaningful for promotions (see is_promotion()); KING means "none". A pawn
  // on the 7th rank thus has four different moves to the same square.
  piece_type promotion = KING;

  // Same squares, and for promotions the same promotion piece.
  bool operator==(const move& other) const {
    return from_x == other.from_x && from_y == other.from_y &&
           to_x == other.to_x && to_y == other.to_y &&
           promotion == other.promotion;
  }

  bool operator!=(const move& other) const { return !(*this == other); }
};

// Moves are copied byte by byte (move lists, later fixed stacks for the
// search and the Game Boy Advance): no member may need a copy constructor.
static_assert(std::is_trivially_copyable<move>::value,
              "move must stay trivially copyable");

/** True for promotions (with or without capture). */
inline bool is_promotion(const move& m) {
  return m.type == PROMOTION || m.type == CAPTURING_PROMOTION;
}

/**
 * Formats a move in UCI notation: start and target square, e.g. "e2e4",
 * plus the promotion piece for promotions ("e7e8q", "e7e8n").
 */
inline std::string to_uci(const move& m) {
  std::string s;
  s += static_cast<char>('a' + m.from_x);
  s += static_cast<char>('1' + m.from_y);
  s += static_cast<char>('a' + m.to_x);
  s += static_cast<char>('1' + m.to_y);
  if (is_promotion(m)) s += "kqbnrp"[m.promotion];  // order of piece_type
  return s;
}

#endif
