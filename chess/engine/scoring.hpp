#pragma once

#include "../datastructures/board.hpp"
#include "../datastructures/enums.hpp"
#include "../datastructures/piece.hpp"

/**
 * The evaluation for the search (minimax): how good a position is for one
 * side, in centipawns (hundredths of a pawn, so a pawn is 100). Whole
 * numbers only, no floating point: cheap, exact, and possible on the Game
 * Boy Advance (no floating point unit).
 *
 * The score is material plus a bonus or malus for the square each piece
 * stands on (piece-square tables): a knight in the center is worth more than
 * a knight on the edge, a pawn is worth more the further it has advanced, a
 * king is safer behind its pawns than in the middle of the board.
 */
namespace scoring {

/** Material in centipawns: pawn 100, knight 320, bishop 330, rook 500,
 *  queen 900. The king has no material value (it is never captured). */
constexpr int centipawns(piece_type t) {
  switch (t) {
    case PAWN:
      return 100;
    case KNIGHT:
      return 320;
    case BISHOP:
      return 330;
    case ROOK:
      return 500;
    case QUEEN:
      return 900;
    case KING:
      return 0;
  }
  return 0;
}

/**
 * Bonus (or malus, if negative) in centipawns for piece p on square (x, y).
 * The tables are written for White; a black piece uses the mirrored square,
 * so both colors are treated alike.
 */
int square_bonus(piece p, int x, int y);

/**
 * The position from the view of side: its material and square bonuses minus
 * those of the opponent. 0 = even, positive = good for side. The score for
 * White is always minus the score for Black.
 */
int score(const board& b, piece_color side);

}  // namespace scoring
