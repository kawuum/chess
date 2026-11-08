#include "engine.hpp"

#include <assert.h>

#include <algorithm>
#include <stdexcept>
#include <string>
#include <utility>

#include "draw_rules.hpp"
#include "move_generation.hpp"

namespace {

const char* const kStartFen =
    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

bool is_space(char c) {
  return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

// Splits text at spaces into fields. Returns how many fields there are; at
// most 6 are stored, more are only counted (7 means "too many").
int split_fields(std::string_view text, std::string_view (&fields)[6]) {
  int count = 0;
  size_t i = 0;
  while (true) {
    while (i < text.size() && is_space(text[i])) ++i;
    if (i == text.size()) return count;
    if (count == 6) return 7;
    size_t end = i;
    while (end < text.size() && !is_space(text[end])) ++end;
    fields[count++] = text.substr(i, end - i);
    i = end;
  }
}

// Reads a number of 1 to 9 decimal digits (no sign), so it fits in an int.
bool read_number(std::string_view text, int& value) {
  if (text.empty() || text.size() > 9) return false;
  value = 0;
  for (char c : text) {
    if (c < '0' || c > '9') return false;
    value = value * 10 + (c - '0');
  }
  return true;
}

// Maps a FEN piece letter (KQRBNP / kqrbnp) to a piece; false for any other
// character.
bool read_piece(char c, piece& result) {
  const bool white = c >= 'A' && c <= 'Z';
  const piece_color color = white ? piece_color::WHITE : piece_color::BLACK;
  switch (white ? static_cast<char>(c - 'A' + 'a') : c) {
    case 'k':
      result = piece(KING, color);
      return true;
    case 'q':
      result = piece(QUEEN, color);
      return true;
    case 'r':
      result = piece(ROOK, color);
      return true;
    case 'b':
      result = piece(BISHOP, color);
      return true;
    case 'n':
      result = piece(KNIGHT, color);
      return true;
    case 'p':
      result = piece(PAWN, color);
      return true;
  }
  return false;
}

// Field 1, the pieces: rank 8 first, files a to h, digits = empty squares.
bool read_placement(std::string_view text, board& b) {
  int x = 0, y = 7;
  for (char c : text) {
    piece p;
    if (c == '/') {
      if (x != 8 || y == 0) return false;
      x = 0;
      --y;
    } else if (c >= '1' && c <= '8') {
      x += c - '0';
      if (x > 8) return false;
    } else if (x < 8 && read_piece(c, p)) {
      b.add_piece(p, x, y);
      ++x;
    } else {
      return false;
    }
  }
  return y == 0 && x == 8;
}

// Field 3, castling rights: "-" or letters from KQkq.
bool read_castling(std::string_view text, castling_rights (&rights)[2]) {
  if (text == "-") return true;
  int r[2] = {NONE, NONE};
  for (char c : text) {
    switch (c) {
      case 'K':
        r[(int)piece_color::WHITE] |= SHORT;
        break;
      case 'Q':
        r[(int)piece_color::WHITE] |= LONG;
        break;
      case 'k':
        r[(int)piece_color::BLACK] |= SHORT;
        break;
      case 'q':
        r[(int)piece_color::BLACK] |= LONG;
        break;
      default:
        return false;
    }
  }
  rights[0] = static_cast<castling_rights>(r[0]);
  rights[1] = static_cast<castling_rights>(r[1]);
  return true;
}

bool has_piece(const board& b, int x, int y, piece_type type,
               piece_color color) {
  const piece p = b.get_piece(x, y);
  return p.is_valid() && p.get_piece_type() == type &&
         p.get_piece_color() == color;
}

// The position checks after the syntax: things that cannot happen in a game
// and that the move generator relies on.
fen_error check_position(game_history& state) {
  board& b = state.curr_board;
  int kings[2] = {0, 0};
  int king_x[2] = {0, 0}, king_y[2] = {0, 0};
  bool pawn_on_back_rank = false;
  for (int y = 0; y < 8; ++y) {
    for (int x = 0; x < 8; ++x) {
      const piece p = b.get_piece(x, y);
      if (!p.is_valid()) continue;
      const int color = static_cast<int>(p.get_piece_color());
      if (p.get_piece_type() == KING) {
        ++kings[color];
        king_x[color] = x;
        king_y[color] = y;
      } else if (p.get_piece_type() == PAWN && (y == 0 || y == 7)) {
        pawn_on_back_rank = true;
      }
    }
  }
  if (kings[0] != 1 || kings[1] != 1) return fen_error::KINGS;
  if (pawn_on_back_rank) return fen_error::PAWN_ON_BACK_RANK;

  // A castling right needs the king on e1/e8 and the rook in its corner.
  for (piece_color color : {piece_color::WHITE, piece_color::BLACK}) {
    const int rights = state.castlingrights[static_cast<int>(color)];
    const int y = color == piece_color::WHITE ? 0 : 7;
    if (rights != NONE && !has_piece(b, 4, y, KING, color)) {
      return fen_error::CASTLING_PIECES;
    }
    if ((rights & SHORT) && !has_piece(b, 7, y, ROOK, color)) {
      return fen_error::CASTLING_PIECES;
    }
    if ((rights & LONG) && !has_piece(b, 0, y, ROOK, color)) {
      return fen_error::CASTLING_PIECES;
    }
  }

  // An en passant square lies behind a pawn of the side that just moved,
  // which came from the square in front of it: both squares are empty.
  const int ep_x = std::get<0>(state.ep_square);
  const int ep_y = std::get<1>(state.ep_square);
  if (ep_x != 0 || ep_y != 0) {
    const bool white_to_move = state.to_move == piece_color::WHITE;
    const piece_color moved =
        white_to_move ? piece_color::BLACK : piece_color::WHITE;
    const int step = white_to_move ? -1 : 1;  // from ep square to the pawn
    if (ep_y != (white_to_move ? 5 : 2) || b.get_piece(ep_x, ep_y).is_valid() ||
        b.get_piece(ep_x, ep_y - step).is_valid() ||
        !has_piece(b, ep_x, ep_y + step, PAWN, moved)) {
      return fen_error::EN_PASSANT;
    }
  }

  // The side that just moved must not have left its king in check.
  const int waiting = static_cast<int>(state.to_move == piece_color::WHITE
                                           ? piece_color::BLACK
                                           : piece_color::WHITE);
  if (move_generation::is_square_attacked(b, king_x[waiting], king_y[waiting],
                                          state.to_move)) {
    return fen_error::OPPONENT_IN_CHECK;
  }
  return fen_error::NONE;
}

// Reads a FEN string into state, e.g. for the start position:
// "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
fen_error read_fen(std::string_view fen, game_history& state) {
  std::string_view field[6];
  const int fields = split_fields(fen, field);
  if (fields < 4 || fields > 6) return fen_error::FIELDS;

  if (!read_placement(field[0], state.curr_board)) return fen_error::PLACEMENT;

  if (field[1] != "w" && field[1] != "b") return fen_error::SIDE_TO_MOVE;
  state.to_move = field[1] == "w" ? piece_color::WHITE : piece_color::BLACK;

  if (!read_castling(field[2], state.castlingrights)) {
    return fen_error::CASTLING;
  }

  // (0, 0) means no en passant square.
  if (field[3] != "-") {
    const std::string_view ep = field[3];
    if (ep.size() != 2 || ep[0] < 'a' || ep[0] > 'h' || ep[1] < '1' ||
        ep[1] > '8') {
      return fen_error::EN_PASSANT;
    }
    state.ep_square = std::make_tuple(ep[0] - 'a', ep[1] - '1');
  }

  // Half moves since the last capture or pawn move (50-move rule). At 100
  // the game is a draw, so larger numbers are stored as 255.
  int halfmove_clock = 0;
  if (fields >= 5 && !read_number(field[4], halfmove_clock)) {
    return fen_error::HALFMOVE_CLOCK;
  }
  state.halfmove_clock = static_cast<uint8_t>(std::min(halfmove_clock, 255));

  // The engine counts half moves from the start of the game.
  int fullmove = 1;
  if (fields == 6 && (!read_number(field[5], fullmove) || fullmove < 1 ||
                      fullmove > engine::kMaxFullmoveNumber)) {
    return fen_error::FULLMOVE_NUMBER;
  }
  state.num_halfmoves =
      (fullmove - 1) * 2 + (state.to_move == piece_color::BLACK ? 1 : 0);

  return check_position(state);
}

}  // namespace

const char* fen_error_message(fen_error error) {
  switch (error) {
    case fen_error::NONE:
      return "FEN: accepted";
    case fen_error::FIELDS:
      return "FEN: needs 4 to 6 fields separated by spaces";
    case fen_error::PLACEMENT:
      return "FEN: the pieces must be 8 ranks of 8 squares (letters KQRBNP, "
             "kqrbnp, digits for empty squares)";
    case fen_error::SIDE_TO_MOVE:
      return "FEN: the side to move must be w or b";
    case fen_error::CASTLING:
      return "FEN: castling rights must be - or letters from KQkq";
    case fen_error::EN_PASSANT:
      return "FEN: the en passant square must be - or the square behind a "
             "pawn that just moved two squares";
    case fen_error::HALFMOVE_CLOCK:
      return "FEN: the half move clock must be a number of 0 or more";
    case fen_error::FULLMOVE_NUMBER:
      return "FEN: the move number must be a number from 1 to 1000000";
    case fen_error::KINGS:
      return "FEN: each side needs exactly one king";
    case fen_error::PAWN_ON_BACK_RANK:
      return "FEN: no pawn may stand on rank 1 or 8";
    case fen_error::CASTLING_PIECES:
      return "FEN: a castling right needs king and rook on their start "
             "squares";
    case fen_error::OPPONENT_IN_CHECK:
      return "FEN: the side that is not to move must not be in check";
  }
  return "FEN: unknown error";
}

fen_error engine::try_new_game(std::string_view fen) {
  auto state = std::make_shared<game_history>();
  const fen_error error = read_fen(fen, *state);
  if (error != fen_error::NONE) return error;  // the current game goes on

  // Generate the legal moves right away: this also sets is_check and result
  // (checkmate or stalemate) for the new position.
  move_generation mg;
  std::vector<move> moves = mg.generate_all_moves(state.get());
  // Only now, with everything ready, replace the current game.
  this->gh = std::move(state);
  this->legal_moves = std::move(moves);
  this->legal_moves_history.clear();
  apply_draw_rules();
  return fen_error::NONE;
}

void engine::new_game() { new_game(kStartFen); }

void engine::new_game(std::string fen) {
  const fen_error error = try_new_game(fen);
  if (error != fen_error::NONE) {
    throw std::invalid_argument(std::string(fen_error_message(error)) + ": \"" +
                                fen + "\"");
  }
}

void engine::apply_draw_rules() {
  if (this->gh->result != RUNNING) return;  // checkmate or stalemate
  draw_reason reason = draw_rules::applying_rule(*this->gh);
  if (reason == NO_DRAW) return;
  this->gh->result = DRAW;
  this->gh->draw = reason;
  // The game is over: no more moves, like after checkmate or stalemate.
  this->legal_moves.clear();
}

void engine::ensure_legal_moves() {
  // The legal moves of the current position are cached in legal_moves. After
  // undo_move() the cache is empty and is filled again on first use. When the
  // game is over there are no legal moves, so nothing has to be generated.
  if (this->legal_moves.empty() && this->gh->result == RUNNING) {
    move_generation mg;
    this->legal_moves = mg.generate_all_moves(this->gh.get());
  }
}

std::vector<move> engine::get_legal_moves(uint8_t from_x, uint8_t from_y) {
  ensure_legal_moves();
  std::vector<move> possible_moves;
  std::copy_if(this->legal_moves.begin(), this->legal_moves.end(),
               std::back_inserter(possible_moves),
               [from_x, from_y](const move& item) {
                 return (item.from_x == from_x && item.from_y == from_y);
               });
  return possible_moves;
}

std::vector<move> engine::get_all_legal_moves(game_history* gh) {
  if (*gh == *(this->gh.get())) {
    // the current position: use the cache
    ensure_legal_moves();
    return this->legal_moves;
  }
  // some other position: generate its moves
  move_generation mg;
  return mg.generate_all_moves(gh);
}

void engine::perform_move(const move& m) { apply_move(find_legal_move(m)); }

move engine::find_legal_move(const move& m) {
  ensure_legal_moves();
  for (const move& legal : this->legal_moves) {
    if (legal.from_x == m.from_x && legal.from_y == m.from_y &&
        legal.to_x == m.to_x && legal.to_y == m.to_y &&
        (!is_promotion(legal) || legal.promotion == m.promotion)) {
      // Return the engine's own move, not the caller's: mover and type come
      // from the move generator, so a caller with wrong data can do no harm.
      return legal;
    }
  }
  throw std::invalid_argument("illegal move: " + to_uci(m));
}

void engine::apply_move(move m) {
  // m comes from find_legal_move(), so it is legal, including the promotion
  // piece (queen, rook, bishop or knight).
  piece mover = m.mover;

  const uint8_t from_x = m.from_x, from_y = m.from_y;
  const uint8_t to_x = m.to_x, to_y = m.to_y;
  const move_type mt = m.type;

  std::shared_ptr<game_history> new_gm = std::make_shared<game_history>();

  if (mover.get_piece_type() == PAWN && (std::abs(from_y - to_y) == 2)) {
    // A pawn is getting moved. Since this is a double jump, we need to add an
    // en passant square.
    new_gm->ep_square = std::make_tuple(
        to_x,
        (mover.get_piece_color() == piece_color::WHITE ? to_y - 1 : to_y + 1));
  }

  new_gm->curr_board = this->gh->curr_board;
  new_gm->curr_board.move_piece(mover, from_x, from_y, to_x, to_y);
  if (mt == CASTLING) {
    // also move rook
    if (from_x > to_x) {
      // long castle
      new_gm->curr_board.move_piece(piece(ROOK, mover.get_piece_color()), 0,
                                    from_y, 3, to_y);
    } else {
      // short castle
      new_gm->curr_board.move_piece(piece(ROOK, mover.get_piece_color()), 7,
                                    from_y, 5, to_y);
    }
  } else if (is_promotion(m)) {
    new_gm->curr_board.remove_piece(mover, to_x, to_y);
    new_gm->curr_board.add_piece(piece(m.promotion, mover.get_piece_color()),
                                 to_x, to_y);
  }
  piece captured_piece = this->gh->curr_board.get_piece(to_x, to_y);
  if (captured_piece.is_valid() && mt != ENPASSANT) {
    new_gm->curr_board.remove_piece(captured_piece, to_x, to_y);
  } else if (mt == ENPASSANT) {
    piece ep_piece = this->gh->curr_board.get_piece(
        std::get<0>(this->gh->ep_square), from_y);
    new_gm->curr_board.remove_piece(ep_piece, std::get<0>(this->gh->ep_square),
                                    from_y);
  }
  this->gh->performed_move = m;
  new_gm->num_halfmoves = this->gh->num_halfmoves + 1;
  // 50-move rule: a capture or a pawn move starts counting again.
  const bool capture = captured_piece.is_valid() || mt == ENPASSANT;
  if (capture || mover.get_piece_type() == PAWN) {
    new_gm->halfmove_clock = 0;
  } else if (this->gh->halfmove_clock < 255) {
    new_gm->halfmove_clock = this->gh->halfmove_clock + 1;
  } else {
    new_gm->halfmove_clock = 255;
  }
  new_gm->castlingrights[0] = this->gh->castlingrights[0];
  new_gm->castlingrights[1] = this->gh->castlingrights[1];
  // Castling rights. A king move loses both rights of its side. Any move that
  // starts or ends on a corner square loses the right that belongs to that
  // corner: either the rook moved away or it was captured there.
  if (mover.get_piece_type() == KING) {
    new_gm->castlingrights[(int)this->gh->to_move] = NONE;
  }
  auto clear_corner = [&new_gm](int x, int y) {
    if ((x != 0 && x != 7) || (y != 0 && y != 7)) return;
    int color = (int)(y == 0 ? piece_color::WHITE : piece_color::BLACK);
    int lost = x == 0 ? LONG : SHORT;
    new_gm->castlingrights[color] =
        static_cast<castling_rights>(new_gm->castlingrights[color] & ~lost);
  };
  clear_corner(from_x, from_y);
  clear_corner(to_x, to_y);
  new_gm->to_move = this->gh->to_move == piece_color::WHITE
                        ? piece_color::BLACK
                        : piece_color::WHITE;
  new_gm->prev = this->gh;

  this->gh = new_gm;
  // Keep the legal moves of the previous position: undo_move() gets them back
  // without generating them again.
  this->legal_moves_history.push_back(std::move(this->legal_moves));
  // Generate the legal moves of the new position right away: this tells us
  // whether the game is over (mate or stalemate), and the next perform_move()
  // can check its move against this list.
  move_generation mg;
  this->legal_moves = mg.generate_all_moves(this->gh.get());
  apply_draw_rules();
}

board& engine::get_current_board() { return this->gh->curr_board; }

void engine::undo_move() {
  if (this->gh->prev == nullptr) return;
  this->gh = this->gh->prev;
  if (!this->legal_moves_history.empty()) {
    this->legal_moves = std::move(this->legal_moves_history.back());
    this->legal_moves_history.pop_back();
  } else {
    this->legal_moves.clear();  // generated again on first use
  }
}

piece_color engine::get_color_to_move() { return this->gh->to_move; }

game_history engine::get_current_gamestate() {
  game_history g = *this->gh;
  return g;
}
