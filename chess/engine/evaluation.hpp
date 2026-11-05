#pragma once

#include "../datastructures/bit_matrix.hpp"
#include "../datastructures/board.hpp"
#include "../datastructures/enums.hpp"

/**
 * Building blocks for evaluation functions: material, covered squares and
 * threatened pieces. They only look at the board, not at whose turn it is.
 *
 * "Covered" (attacked) means: a piece could capture on that square if an
 * enemy piece stood there. So:
 * - pawns cover the two squares diagonally in front of them, not the square
 *   straight ahead (they cannot capture there);
 * - rooks, bishops and queens cover each line up to and including the first
 *   piece, of either color: a covered own piece is a defended piece;
 * - pinned pieces count like all others (the rules of check are ignored);
 * - en passant does not count.
 * This is the same meaning as move_generation::is_square_attacked().
 */
namespace evaluation {

/**
 * Material value in pawns: queen 9, rook 5, bishop 3, knight 3, pawn 1. The
 * king has the value 0: it is never captured, so it is not counted.
 */
constexpr int piece_value(piece_type t) {
  switch (t) {
    case QUEEN:
      return 9;
    case ROOK:
      return 5;
    case BISHOP:
    case KNIGHT:
      return 3;
    case PAWN:
      return 1;
    case KING:
      return 0;
  }
  return 0;
}

/** Sum of the piece values of color c (39 in the start position). */
int material(const board& b, piece_color c);

/**
 * All squares covered by at least one piece of color c, empty or occupied.
 * get_num_bits_set() on the result gives the number of covered squares (22
 * per side in the start position).
 */
bit_matrix attacked_squares(const board& b, piece_color c);

/**
 * Number of pieces of color c that cover square (x, y). On every line only
 * the first piece counts: a queen behind its own rook on the same file is
 * not counted, although it would also cover the square after the rook has
 * captured.
 */
int attackers(const board& b, int x, int y, piece_color c);

/**
 * Pieces of the opponent of by that are covered by by, i.e. that by could
 * capture. A king in check is included (it is attacked), but has the value
 * 0 in threatened_value().
 */
bit_matrix threatened_pieces(const board& b, piece_color by);

/** Sum of the piece values of threatened_pieces(b, by). */
int threatened_value(const board& b, piece_color by);

/** Pieces of color c that are covered by another piece of color c. */
bit_matrix defended_pieces(const board& b, piece_color c);

}  // namespace evaluation
