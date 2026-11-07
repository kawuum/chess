#include "scoring.hpp"

namespace scoring {

namespace {

// Piece-square tables, written as the board looks from White's side: the
// first row is rank 8, the last row rank 1, files a to h from left to right.
// Values in centipawns.

constexpr int kPawn[8][8] = {
    {0, 0, 0, 0, 0, 0, 0, 0},          // rank 8 (a pawn never stands here)
    {60, 60, 60, 60, 60, 60, 60, 60},  // rank 7: one step from promotion
    {15, 15, 25, 35, 35, 25, 15, 15},
    {5, 5, 10, 25, 25, 10, 5, 5},
    {0, 0, 5, 20, 20, 5, 0, 0},  // rank 4: center pawns
    {5, 0, -5, 5, 5, -5, 0, 5},
    {5, 10, 10, -15, -15, 10, 10, 5},  // rank 2: d/e pawns should move
    {0, 0, 0, 0, 0, 0, 0, 0},          // rank 1 (never)
};

constexpr int kKnight[8][8] = {
    {-50, -35, -30, -30, -30, -30, -35, -50},
    {-35, -15, 0, 5, 5, 0, -15, -35},
    {-30, 5, 15, 20, 20, 15, 5, -30},
    {-30, 5, 20, 25, 25, 20, 5, -30},
    {-30, 0, 20, 25, 25, 20, 0, -30},
    {-30, 5, 15, 15, 15, 15, 5, -30},
    {-35, -15, 0, 5, 5, 0, -15, -35},
    {-50, -35, -30, -30, -30, -30, -35, -50},  // knights on the edge are weak
};

constexpr int kBishop[8][8] = {
    {-20, -10, -10, -10, -10, -10, -10, -20},
    {-10, 0, 0, 0, 0, 0, 0, -10},
    {-10, 0, 5, 10, 10, 5, 0, -10},
    {-10, 5, 5, 10, 10, 5, 5, -10},
    {-10, 0, 10, 10, 10, 10, 0, -10},
    {-10, 10, 10, 10, 10, 10, 10, -10},
    {-10, 5, 0, 0, 0, 0, 5, -10},
    {-20, -10, -10, -10, -10, -10, -10, -20},
};

constexpr int kRook[8][8] = {
    {0, 0, 0, 0, 0, 0, 0, 0},
    {10, 15, 15, 15, 15, 15, 15, 10},  // rank 7: attacks the pawns
    {-5, 0, 0, 0, 0, 0, 0, -5},
    {-5, 0, 0, 0, 0, 0, 0, -5},
    {-5, 0, 0, 0, 0, 0, 0, -5},
    {-5, 0, 0, 0, 0, 0, 0, -5},
    {-5, 0, 0, 0, 0, 0, 0, -5},
    {0, 0, 0, 5, 5, 0, 0, 0},  // central files after castling
};

constexpr int kQueen[8][8] = {
    {-20, -10, -10, -5, -5, -10, -10, -20},
    {-10, 0, 0, 0, 0, 0, 0, -10},
    {-10, 0, 5, 5, 5, 5, 0, -10},
    {-5, 0, 5, 5, 5, 5, 0, -5},
    {-5, 0, 5, 5, 5, 5, 0, -5},
    {-10, 5, 5, 5, 5, 5, 0, -10},
    {-10, 0, 5, 0, 0, 0, 0, -10},
    {-20, -10, -10, -5, -5, -10, -10, -20},
};

// The king belongs behind its pawns while there are pieces on the board: in
// a corner after castling it gets a bonus, in the middle a malus. (An
// endgame table, where the king should come to the center, is a possible
// later improvement.)
constexpr int kKing[8][8] = {
    {-30, -40, -40, -50, -50, -40, -40, -30},
    {-30, -40, -40, -50, -50, -40, -40, -30},
    {-30, -40, -40, -50, -50, -40, -40, -30},
    {-30, -40, -40, -50, -50, -40, -40, -30},
    {-20, -30, -30, -40, -40, -30, -30, -20},
    {-10, -20, -20, -20, -20, -20, -20, -10},
    {15, 15, 0, 0, 0, 0, 15, 15},
    {20, 30, 10, 0, 0, 10, 30, 20},  // castled king: g1 or b1/c1
};

const int (*table_for(piece_type t))[8] {
  switch (t) {
    case PAWN:
      return kPawn;
    case KNIGHT:
      return kKnight;
    case BISHOP:
      return kBishop;
    case ROOK:
      return kRook;
    case QUEEN:
      return kQueen;
    case KING:
      return kKing;
  }
  return kPawn;
}

}  // namespace

int square_bonus(piece p, int x, int y) {
  // The tables show rank 8 in row 0. A white piece on rank y is in row 7 - y.
  // A black piece is looked up on the mirrored square (rank 7 - y), so its
  // row is y: a black pawn on rank 2 is "one step from promotion" too.
  const int row = p.get_piece_color() == piece_color::WHITE ? 7 - y : y;
  return table_for(p.get_piece_type())[row][x];
}

int score(const board& b, piece_color side) {
  int total = 0;
  for (int y = 0; y < 8; ++y) {
    for (int x = 0; x < 8; ++x) {
      const piece p = b.get_piece(x, y);
      if (!p.is_valid()) continue;
      const int value = centipawns(p.get_piece_type()) + square_bonus(p, x, y);
      total += p.get_piece_color() == side ? value : -value;
    }
  }
  return total;
}

}  // namespace scoring
