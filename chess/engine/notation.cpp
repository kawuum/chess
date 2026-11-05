#include "notation.hpp"

#include <vector>

namespace {

char piece_letter(piece_type t) {
  switch (t) {
    case KING:
      return 'K';
    case QUEEN:
      return 'Q';
    case ROOK:
      return 'R';
    case BISHOP:
      return 'B';
    case KNIGHT:
      return 'N';
    case PAWN:
      return ' ';  // pawns have no letter
  }
  return '?';
}

char file_char(int x) { return static_cast<char>('a' + x); }
char rank_char(int y) { return static_cast<char>('1' + y); }

// "b", "1", "a1" or "": what has to be added after the piece letter so the
// move cannot be confused with a move of another piece of the same kind to
// the same square (e.g. "Nbd2" when knights b1 and f1 can both go to d2).
std::string disambiguation(const move& m, const std::vector<move>& legal) {
  bool other = false, same_file = false, same_rank = false;
  for (const move& o : legal) {
    if (o.to_x != m.to_x || o.to_y != m.to_y) continue;          // other target
    if (o.from_x == m.from_x && o.from_y == m.from_y) continue;  // m itself
    // Another kind of piece (all legal moves belong to the same color).
    if (o.mover.get_piece_type() != m.mover.get_piece_type()) continue;
    other = true;
    if (o.from_x == m.from_x) same_file = true;
    if (o.from_y == m.from_y) same_rank = true;
  }
  if (!other) return "";
  if (!same_file) return std::string(1, file_char(m.from_x));  // "Nbd2"
  if (!same_rank) return std::string(1, rank_char(m.from_y));  // "R1a3"
  return {file_char(m.from_x), rank_char(m.from_y)};           // "Qa1b2"
}

}  // namespace

std::string to_san(engine& e, const move& m) {
  game_history before = e.get_current_gamestate();
  const std::vector<move> legal = e.get_all_legal_moves(&before);
  const piece_type type = m.mover.get_piece_type();

  std::string san;
  if (m.type == CASTLING) {
    san = m.to_x > m.from_x ? "O-O" : "O-O-O";  // king side or queen side
  } else {
    const bool capture =
        before.curr_board.get_piece(m.to_x, m.to_y).is_valid() ||
        m.type == ENPASSANT;
    if (type == PAWN) {
      if (capture) san += file_char(m.from_x);  // "exd5"
    } else {
      san += piece_letter(type);
      san += disambiguation(m, legal);
    }
    if (capture) san += 'x';
    san += file_char(m.to_x);
    san += rank_char(m.to_y);
    if (is_promotion(m)) {
      san += '=';
      san += piece_letter(m.promotion);
    }
  }

  // Check or mate: play the move, look, take it back.
  e.perform_move(m);
  game_history after = e.get_current_gamestate();
  e.undo_move();
  if (after.result == WHITE_WIN || after.result == BLACK_WIN) {
    san += '#';
  } else if (after.is_check) {
    san += '+';
  }
  return san;
}
