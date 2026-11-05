#include "move_generation.hpp"

#include <cassert>

#include "../datastructures/enums.hpp"

std::vector<move> move_generation::generate_all_moves(game_history* gh) {
  this->gamestate = *gh;
  std::vector<move> res =
      generate_all_moves(gh->curr_board, gh->to_move, false);
  // A "null move" of the king: is_check() then just tells whether the side to
  // move is in check right now.
  move null_move = {piece(KING, gh->to_move), 0, 0, 0, 0, NO_MOVE};
  gh->is_check = is_check(gh->curr_board, null_move);
  // No legal moves means the game is over: checkmate if in check, otherwise
  // stalemate.
  if (res.empty()) {
    if (gh->is_check) {
      gh->result = gh->to_move == piece_color::WHITE ? BLACK_WIN : WHITE_WIN;
    } else {
      gh->result = DRAW;
    }
  }
  return res;
}

std::vector<move> move_generation::generate_all_moves(board& b,
                                                      piece_color to_move,
                                                      bool check_for_check) {
  std::vector<move> moves;
  moves.reserve(80);  // too many, but who cares... ;-)

  bit_matrix bm = b.get_bit_matrix_for_color(to_move);
  for (uint8_t y = 0; y < 8; ++y) {
    for (uint8_t x = 0; x < 8; ++x) {
      if (bm.get_bit_at(x, y)) {
        piece p = b.get_piece(x, y);
        if (!p.is_valid()) continue;
        std::vector<move> piece_moves =
            generate_moves(p, x, y, b, check_for_check);

        if (check_for_check && !piece_moves.empty()) {
          return piece_moves;
        } else if (!piece_moves.empty())
          moves.insert(moves.end(), piece_moves.begin(), piece_moves.end());
      }
    }
  }

  return moves;
}

std::vector<move> move_generation::generate_moves(
    piece mover, uint8_t from_x, uint8_t from_y,
    std::shared_ptr<game_history> gh) {
  assert(mover.is_valid());
  assert(mover.get_piece_color() == gh->to_move);
  this->gamestate = *gh;
  return generate_moves(mover, from_x, from_y, gh->curr_board, false);
}

std::vector<move> move_generation::generate_moves(piece mover, uint8_t from_x,
                                                  uint8_t from_y, board& b,
                                                  bool check_for_check) {
  assert(mover.is_valid());
  switch (mover.get_piece_type()) {
    case KNIGHT: {
      std::vector<move> moves;
      moves.reserve(8);
      knightmoves(mover, from_x, from_y, b, moves, check_for_check);
      return moves;
      break;
    }
    case KING: {
      std::vector<move> moves;
      moves.reserve(8);
      kingmoves(mover, from_x, from_y, b, moves, check_for_check);
      return moves;
      break;
    }
    case BISHOP: {
      std::vector<move> moves;
      moves.reserve(14);
      bishopmoves(mover, from_x, from_y, b, moves, check_for_check);
      return moves;
      break;
    }
    case ROOK: {
      std::vector<move> moves;
      moves.reserve(14);
      rookmoves(mover, from_x, from_y, b, moves, check_for_check);
      return moves;
      break;
    }
    case PAWN: {
      std::vector<move> moves;
      moves.reserve(4);
      pawnmoves(mover, from_x, from_y, b, moves, check_for_check);
      return moves;
      break;
    }
    case QUEEN: {
      std::vector<move> moves;
      moves.reserve(28);
      queenmoves(mover, from_x, from_y, b, moves, check_for_check);
      return moves;
      break;
    }
    default:
      assert(false);
      return std::vector<move>();
      break;
  }
}

void move_generation::knightmoves(piece mover, uint8_t from_x, uint8_t from_y,
                                  board& b, std::vector<move>& moves,
                                  bool check_for_check) {
  assert(mover.get_piece_type() == KNIGHT);
  for (int8_t x = -2; x <= 2; ++x) {
    if (from_x + x < 0 || from_x + x > 7 || x == 0) continue;
    for (int8_t y = -2; y <= 2; ++y) {
      if ((from_y + y < 0) || (from_y + y > 7) || (y == 0) ||
          (std::abs(x % 2) == std::abs(y % 2)))
        continue;
      // check occupancy, if not occupied by friendly piece, add move to vector
      auto p = b.get_piece(from_x + x, from_y + y);
      move m = (move){
          mover, from_x, from_y, (uint8_t)(from_x + x), (uint8_t)(from_y + y),
          MOVE};
      if (p.is_valid() && (p.get_piece_color() != mover.get_piece_color())) {
        m.type = CAPTURE;
        if (check_for_check && p.get_piece_type() == KING) {
          // doesn't matter that this is a capturing move and we return a
          // standard move, we just want to find the first king capture
          moves.push_back(m);
          return;
        } else if (!check_for_check && !is_check(b, m))
          moves.push_back(m);
      } else if (!p.is_valid()) {
        if (!check_for_check && !is_check(b, m)) moves.push_back(m);
      }
    }
  }
}

void move_generation::kingmoves(piece mover, uint8_t from_x, uint8_t from_y,
                                board& b, std::vector<move>& moves,
                                bool check_for_check) {
  assert(mover.get_piece_type() == KING);
  for (int8_t x = -1; x <= 1; ++x) {
    if (from_x + x < 0 || from_x + x > 7) continue;
    for (int8_t y = -1; y <= 1; ++y) {
      if (from_y + y < 0 || from_y + y > 7 || (y == 0 && x == 0)) continue;
      // check occupancy, if not occupied by friendly piece, add move to vector
      auto p = b.get_piece(from_x + x, from_y + y);
      if ((p.is_valid() && p.get_piece_color() != mover.get_piece_color())) {
        move m = (move){mover,
                        from_x,
                        from_y,
                        (uint8_t)(from_x + x),
                        (uint8_t)(from_y + y),
                        CAPTURE};
        if (check_for_check && p.get_piece_type() == KING) {
          moves.push_back(m);
          return;
        } else if (!check_for_check && !is_check(b, m))
          moves.push_back(m);
      } else if (!p.is_valid()) {
        move m = (move){
            mover, from_x, from_y, (uint8_t)(from_x + x), (uint8_t)(from_y + y),
            MOVE};
        if (!check_for_check && !is_check(b, m)) moves.push_back(m);
      }
    }
  }

  // check castling options
  castling_rights cr =
      this->gamestate.castlingrights[(int)mover.get_piece_color()];
  if (this->gamestate.castlingrights[(int)mover.get_piece_color()] != NONE &&
      !check_for_check) {
    if (cr == BOTH || cr == SHORT) {
      // check if short castling is available
      // 1. no piece between king and rook
      // 2. none of the squares the king touches are under attack (king not in
      // check before/after and squares between from and to square not under
      // attack)
      for (uint8_t x = from_x; x <= 7; ++x) {
        if (x == from_x) {
          // originating square, only check for check
          if (!check_for_check &&
              is_check(b, (move){mover, from_x, from_y, x, from_y, MOVE}))
            break;
        } else if (x == 7) {
          // rook square: the castling right says the rook never moved, but
          // make sure it is really there (it may have been captured)
          piece rook = b.get_piece(x, from_y);
          if (!rook.is_valid() || rook.get_piece_type() != ROOK) break;
          moves.push_back((move){mover, from_x, from_y, (uint8_t)(from_x + 2),
                                 from_y, CASTLING});
        } else {
          // in between squares... check if empty and for check
          if (b.get_piece(x, from_y).is_valid()) break;
          if (!check_for_check &&
              is_check(b, (move){mover, from_x, from_y, x, from_y, MOVE}))
            break;
        }
      }
    }
    if (cr == BOTH || cr == LONG) {
      // check if long castling is available
      // 1. no piece between king and rook
      // 2. none of the squares the king touches are under attack (king not in
      // check before/after and squares between from and to square not under
      // attack)
      for (uint8_t x = from_x; x >= 1; --x) {
        if (x == from_x) {
          // originating square, only check for check
          if (!check_for_check &&
              is_check(b, (move){mover, from_x, from_y, x, from_y, MOVE}))
            break;
        } else if (x == 1) {
          // b-file: the king never crosses it, so it may be attacked, but the
          // rook passes it, so it must be empty
          if (b.get_piece(x, from_y).is_valid()) break;
          // a-file: the rook must still be there (it may have been captured)
          piece rook = b.get_piece(0, from_y);
          if (!rook.is_valid() || rook.get_piece_type() != ROOK) break;
          moves.push_back((move){mover, from_x, from_y, (uint8_t)(from_x - 2),
                                 from_y, CASTLING});
        } else {
          // in between squares... check if empty and for check
          if (b.get_piece(x, from_y).is_valid()) break;
          if (!check_for_check &&
              is_check(b, (move){mover, from_x, from_y, x, from_y, MOVE}))
            break;
        }
      }
    }
  }
}

void move_generation::bishopmoves(piece mover, uint8_t from_x, uint8_t from_y,
                                  board& b, std::vector<move>& moves,
                                  bool check_for_check) {
  assert(mover.get_piece_type() == BISHOP || mover.get_piece_type() == QUEEN);

  bool stopped[4] = {
      false}; /* upper-left, lower-left, upper-right, lower-right */
  int8_t x_diff = 1;
  int8_t y_diff = 1;
  while (
      ((((int8_t)from_x) - x_diff >= 0) && (((int8_t)from_x) + x_diff <= 7)) ||
      ((((int8_t)from_x) - x_diff >= 0) && (((int8_t)from_x) - x_diff >= 0)) ||
      ((((int8_t)from_x) + x_diff <= 7) && (((int8_t)from_x) + x_diff <= 7)) ||
      ((((int8_t)from_x) + x_diff <= 7) && (((int8_t)from_x) - x_diff >= 0))) {
    // upper-left
    bishopmoves_helper(mover, b, &stopped[0], from_x, from_y,
                       (int8_t)from_x - x_diff, (int8_t)from_y + y_diff, moves,
                       check_for_check);
    if (check_for_check && !moves.empty()) return;

    // lower-left
    bishopmoves_helper(mover, b, &stopped[1], from_x, from_y,
                       (int8_t)from_x - x_diff, (int8_t)from_y - y_diff, moves,
                       check_for_check);
    if (check_for_check && !moves.empty()) return;

    // upper-right
    bishopmoves_helper(mover, b, &stopped[2], from_x, from_y,
                       (int8_t)from_x + x_diff, (int8_t)from_y + y_diff, moves,
                       check_for_check);
    if (check_for_check && !moves.empty()) return;

    // lower-right
    bishopmoves_helper(mover, b, &stopped[3], from_x, from_y,
                       (int8_t)from_x + x_diff, (int8_t)from_y - y_diff, moves,
                       check_for_check);
    if (check_for_check && !moves.empty()) return;

    ++x_diff;
    ++y_diff;
  }
}

void move_generation::bishopmoves_helper(piece& mover, board& b, bool* stopped,
                                         uint8_t from_x, uint8_t from_y,
                                         int8_t x_diff, int8_t y_diff,
                                         std::vector<move>& moves,
                                         bool check_for_check) {
  if (!*stopped && x_diff >= 0 && x_diff <= 7 && y_diff >= 0 && y_diff <= 7) {
    piece p = b.get_piece(x_diff, y_diff);
    if (p.is_valid()) {
      piece_color p_col = p.get_piece_color();
      if (p_col != mover.get_piece_color()) {
        move m = (move){mover,           from_x,          from_y,
                        (uint8_t)x_diff, (uint8_t)y_diff, CAPTURE};
        if (check_for_check && p.get_piece_type() == KING) {
          moves.push_back(m);
          return;
        }
        if (!check_for_check && !is_check(b, m)) moves.push_back(m);
      }
      *stopped = true;
    } else {
      move m =
          (move){mover, from_x, from_y, (uint8_t)x_diff, (uint8_t)y_diff, MOVE};
      if (!check_for_check && !is_check(b, m)) {
        moves.push_back(m);
      }
    }
  }
}

void move_generation::rookmoves(piece mover, uint8_t from_x, uint8_t from_y,
                                board& b, std::vector<move>& moves,
                                bool check_for_check) {
  assert(mover.get_piece_type() == ROOK || mover.get_piece_type() == QUEEN);

  bool stopped[4] = {false}; /* x-left, x-right, y-down, y-up */
  int8_t x_diff = 1;
  int8_t y_diff = 1;

  while ((((int8_t)from_x) - x_diff >= 0) || (((int8_t)from_x) + x_diff <= 7) ||
         (((int8_t)from_y) - y_diff >= 0) || (((int8_t)from_y) + y_diff <= 7)) {
    // x-left
    rookmoves_helper(mover, b, &stopped[0], from_x, from_y,
                     (int8_t)from_x - x_diff, from_y, moves, check_for_check);
    if (check_for_check && !moves.empty()) return;

    // x-right
    rookmoves_helper(mover, b, &stopped[1], from_x, from_y, from_x + x_diff,
                     from_y, moves, check_for_check);
    if (check_for_check && !moves.empty()) return;

    // y-up
    rookmoves_helper(mover, b, &stopped[2], from_x, from_y, from_x,
                     from_y + y_diff, moves, check_for_check);
    if (check_for_check && !moves.empty()) return;

    // y-down
    rookmoves_helper(mover, b, &stopped[3], from_x, from_y, from_x,
                     (int8_t)from_y - y_diff, moves, check_for_check);
    if (check_for_check && !moves.empty()) return;
    ++x_diff;
    ++y_diff;
  }
}

void move_generation::rookmoves_helper(piece& mover, board& b, bool* stopped,
                                       uint8_t from_x, uint8_t from_y,
                                       int8_t x_diff, int8_t y_diff,
                                       std::vector<move>& moves,
                                       bool check_for_check) {
  if (!*stopped && x_diff >= 0 && x_diff <= 7 && y_diff >= 0 && y_diff <= 7) {
    piece p = b.get_piece(x_diff, y_diff);
    if (p.is_valid()) {
      piece_color p_col = p.get_piece_color();
      if (p_col != mover.get_piece_color()) {
        move m = (move){mover,           from_x,          from_y,
                        (uint8_t)x_diff, (uint8_t)y_diff, CAPTURE};
        if (check_for_check && p.get_piece_type() == KING) {
          moves.push_back(m);
          return;
        }
        if (!check_for_check && !is_check(b, m)) moves.push_back(m);
      }
      *stopped = true;
    } else {
      move m =
          (move){mover, from_x, from_y, (uint8_t)x_diff, (uint8_t)y_diff, MOVE};
      if (!check_for_check && !is_check(b, m)) moves.push_back(m);
    }
  }
}

void move_generation::queenmoves(piece mover, uint8_t from_x, uint8_t from_y,
                                 board& b, std::vector<move>& moves,
                                 bool check_for_check) {
  assert(mover.get_piece_type() == QUEEN);
  rookmoves(mover, from_x, from_y, b, moves, check_for_check);
  if (check_for_check && !moves.empty()) return;
  bishopmoves(mover, from_x, from_y, b, moves, check_for_check);
}

void move_generation::pawnmoves(piece mover, uint8_t from_x, uint8_t from_y,
                                board& b, std::vector<move>& moves,
                                bool check_for_check) {
  assert(mover.get_piece_type() == PAWN);

  int8_t y, starting_y, promotion_y;
  y = mover.get_piece_color() == piece_color::WHITE ? 1 : -1;
  starting_y = mover.get_piece_color() == piece_color::WHITE ? 1 : 6;
  promotion_y = mover.get_piece_color() == piece_color::WHITE ? 7 : 0;

  // check possible capture moves
  // we don't have to boundary check y here, since a pawn on the last file gets
  // promoted (and can't move backwards)
  for (int8_t x = -1; x < 2; x += 2) {
    if ((from_x + x >= 0) && (from_x + x <= 7)) {
      piece p = b.get_piece(from_x + x, from_y + y);
      if (p.is_valid() && p.get_piece_color() != mover.get_piece_color()) {
        move_type mtype =
            from_y + y == promotion_y ? CAPTURING_PROMOTION : CAPTURE;
        move m = (move){
            mover, from_x, from_y, (uint8_t)(from_x + x), (uint8_t)(from_y + y),
            mtype};
        if (check_for_check && p.get_piece_type() == KING)
          moves.push_back(m);
        else if (!check_for_check && !is_check(b, m))
          moves.push_back(m);
      } else if (!((from_y + y) ==
                   0)  // "empty" en passant square needs to be excluded, en
                       // passant captures on 0,0 are impossible; only y-ranks 2
                       // and 5 should be possible
                 && (std::get<0>(this->gamestate.ep_square) == (from_x + x)) &&
                 (std::get<1>(this->gamestate.ep_square) == (from_y + y))) {
        move m = (move){mover,
                        from_x,
                        from_y,
                        (uint8_t)(from_x + x),
                        (uint8_t)(from_y + y),
                        ENPASSANT};
        if (check_for_check && p.get_piece_type() == KING && p.is_valid())
          moves.push_back(m);
        else if (!check_for_check && !is_check(b, m))
          moves.push_back(m);
      }
    }
  }

  // we checked capturing moves, so nothing to do left if checking for king
  // captures
  if (check_for_check) return;

  // check occupancy for tile ahead of piece
  piece p = b.get_piece(from_x, from_y + y);
  if (!p.is_valid()) {
    move_type mtype = from_y + y == promotion_y ? PROMOTION : MOVE;
    move m = (move){
        mover, from_x, from_y, (uint8_t)(from_x), (uint8_t)(from_y + y), mtype};
    if (!is_check(b, m)) {
      // we CANNOT just return if a single move would (still) result in check...
      // we might be able to block a check with a double move!
      moves.push_back(m);
    }
    // check if we are still on the starting file
    if (from_y == starting_y) {
      p = b.get_piece(from_x, from_y + 2 * y);
      if (!p.is_valid()) {
        m = (move){
            mover, from_x, from_y, (uint8_t)(from_x), (uint8_t)(from_y + y * 2),
            MOVE};
        if (!is_check(b, m)) moves.push_back(m);
      }
    }
  }
}
namespace {

bool on_board(int x, int y) { return x >= 0 && x <= 7 && y >= 0 && y <= 7; }

// True if square (x, y) holds a piece of the given color and one of the given
// types.
bool has(board& b, int x, int y, piece_color color, piece_type t1,
         piece_type t2) {
  piece p = b.get_piece(x, y);
  return p.is_valid() && p.get_piece_color() == color &&
         (p.get_piece_type() == t1 || p.get_piece_type() == t2);
}

// Walks from (x, y) in direction (dx, dy) to the first piece and reports
// whether it is an attacker of the given color and one of the given types.
bool attacked_along(board& b, int x, int y, int dx, int dy, piece_color by,
                    piece_type t1, piece_type t2) {
  for (x += dx, y += dy; on_board(x, y); x += dx, y += dy) {
    piece p = b.get_piece(x, y);
    if (!p.is_valid()) continue;  // empty square: keep going
    return p.get_piece_color() == by &&
           (p.get_piece_type() == t1 || p.get_piece_type() == t2);
  }
  return false;
}

}  // namespace

bool move_generation::is_square_attacked(board& b, int x, int y,
                                         piece_color by) {
  // Look outwards from the square instead of generating all moves of the
  // attacker: a piece attacks (x, y) exactly if (x, y) would attack that
  // piece's square with the same move pattern.

  // Knights: the 8 L-shaped jumps.
  static const int kKnight[8][2] = {{1, 2},   {2, 1},   {2, -1}, {1, -2},
                                    {-1, -2}, {-2, -1}, {-2, 1}, {-1, 2}};
  for (const auto& d : kKnight) {
    int nx = x + d[0], ny = y + d[1];
    if (on_board(nx, ny) && has(b, nx, ny, by, KNIGHT, KNIGHT)) return true;
  }

  // Pawns capture diagonally forwards. A white pawn attacks upwards, so it
  // has to stand one rank below the square; a black pawn one rank above.
  int pawn_y = by == piece_color::WHITE ? y - 1 : y + 1;
  for (int dx : {-1, 1}) {
    if (on_board(x + dx, pawn_y) && has(b, x + dx, pawn_y, by, PAWN, PAWN))
      return true;
  }

  // The enemy king on one of the 8 neighbour squares.
  for (int dx = -1; dx <= 1; ++dx) {
    for (int dy = -1; dy <= 1; ++dy) {
      if ((dx != 0 || dy != 0) && on_board(x + dx, y + dy) &&
          has(b, x + dx, y + dy, by, KING, KING))
        return true;
    }
  }

  // Sliding pieces: the first piece on each line decides.
  static const int kStraight[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
  static const int kDiagonal[4][2] = {{1, 1}, {1, -1}, {-1, 1}, {-1, -1}};
  for (const auto& d : kStraight) {
    if (attacked_along(b, x, y, d[0], d[1], by, ROOK, QUEEN)) return true;
  }
  for (const auto& d : kDiagonal) {
    if (attacked_along(b, x, y, d[0], d[1], by, BISHOP, QUEEN)) return true;
  }
  return false;
}

bool move_generation::is_check(board& b, move m) {
  // Castling never arrives here as one move: kingmoves() checks the king's
  // start, transit and target square one by one as normal MOVEs. The CASTLING
  // case below only keeps the function complete.

  board new_board = b;
  piece_color color = m.mover.get_piece_color();
  switch (m.type) {
    case NO_MOVE:
      // "null" move, don't do anything
      break;
    case MOVE:
      new_board.move_piece(m.mover, m.from_x, m.from_y, m.to_x, m.to_y);
      break;
    case CAPTURE:
      new_board.move_piece(m.mover, m.from_x, m.from_y,
                           new_board.get_piece(m.to_x, m.to_y), m.to_x, m.to_y);
      break;
    case CASTLING:
      new_board.move_piece(m.mover, m.from_x, m.from_y, m.to_x,
                           m.to_y);  // move king
      new_board.move_piece(piece(ROOK, color), m.from_x > m.to_x ? 0 : 7,
                           m.from_y, m.from_x > m.to_x ? 3 : 5,
                           m.to_y);  // move rook
      break;
    case ENPASSANT:
      new_board.move_piece(m.mover, m.from_x, m.from_y, m.to_x, m.to_y);
      new_board.remove_piece(new_board.get_piece(m.to_x, m.from_y), m.to_x,
                             m.from_y);  // enemy pawn always is in target X
                                         // file, but in original Y file
      break;
    case PROMOTION:
      new_board.remove_piece(m.mover, m.from_x, m.from_y);
      new_board.add_piece(piece(QUEEN, color), m.to_x,
                          m.to_y);  // actual piece doesn't matter here
      break;
    case CAPTURING_PROMOTION:
      new_board.remove_piece(m.to_x, m.to_y);  // remove captured piece first
      new_board.remove_piece(m.mover, m.from_x, m.from_y);
      new_board.add_piece(piece(QUEEN, color), m.to_x,
                          m.to_y);  // actual piece doesn't matter here
      break;
    default:
      return false;
      break;
  }

  // Find the own king. Its bitboard has exactly one bit set; with the bit
  // order of bit_matrix (a1 = highest bit) the number of leading zero bits is
  // the square index 8 * y + x.
  uint64_t king =
      new_board.get_bit_matrix_for_piece(piece(KING, color)).get_raw();
  if (king == 0) return false;  // no king (only in artificial test positions)
  int index = __builtin_clzll(king);
  piece_color opponent =
      color == piece_color::WHITE ? piece_color::BLACK : piece_color::WHITE;
  return is_square_attacked(new_board, index % 8, index / 8, opponent);
}
