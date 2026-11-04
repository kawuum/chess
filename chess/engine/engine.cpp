#include "engine.hpp"

#include <assert.h>

#include <algorithm>
#include <cctype>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>

#include "move_generation.hpp"

namespace {

const char* const kStartFen =
    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

// Maps a FEN piece letter (KQRBNP / kqrbnp) to a piece.
piece piece_from_fen_char(char c) {
  piece_color color = std::isupper(static_cast<unsigned char>(c))
                          ? piece_color::WHITE
                          : piece_color::BLACK;
  switch (std::tolower(static_cast<unsigned char>(c))) {
    case 'k':
      return piece(KING, color);
    case 'q':
      return piece(QUEEN, color);
    case 'r':
      return piece(ROOK, color);
    case 'b':
      return piece(BISHOP, color);
    case 'n':
      return piece(KNIGHT, color);
    case 'p':
      return piece(PAWN, color);
  }
  throw std::invalid_argument(std::string("FEN: unknown piece '") + c + "'");
}

}  // namespace

void engine::new_game() { new_game(kStartFen); }

void engine::new_game(std::string fen) {
  // A FEN string has six fields separated by spaces, e.g. for the start
  // position: "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
  std::istringstream in(fen);
  std::string placement, side, castling, ep;
  int halfmove_clock = 0, fullmove = 1;
  if (!(in >> placement >> side >> castling >> ep)) {
    throw std::invalid_argument("FEN: needs at least 4 fields: " + fen);
  }
  in >> halfmove_clock >> fullmove;  // optional, defaults stay if missing

  auto state = std::make_shared<game_history>();

  // 1. Piece placement: rank 8 first, files a to h, digits = empty squares.
  int x = 0, y = 7;
  for (char c : placement) {
    if (c == '/') {
      if (x != 8) throw std::invalid_argument("FEN: bad rank in " + fen);
      x = 0;
      --y;
    } else if (c >= '1' && c <= '8') {
      x += c - '0';
    } else {
      if (x > 7 || y < 0) throw std::invalid_argument("FEN: too many squares");
      state->curr_board.add_piece(piece_from_fen_char(c), x, y);
      ++x;
    }
  }
  if (y != 0 || x != 8) {
    throw std::invalid_argument("FEN: placement must have 8 ranks: " + fen);
  }

  // 2. Side to move.
  if (side != "w" && side != "b") {
    throw std::invalid_argument("FEN: side to move must be w or b");
  }
  state->to_move = side == "w" ? piece_color::WHITE : piece_color::BLACK;

  // 3. Castling rights: K/Q for white, k/q for black, "-" for none.
  int rights[2] = {NONE, NONE};
  for (char c : castling) {
    switch (c) {
      case 'K':
        rights[(int)piece_color::WHITE] |= SHORT;
        break;
      case 'Q':
        rights[(int)piece_color::WHITE] |= LONG;
        break;
      case 'k':
        rights[(int)piece_color::BLACK] |= SHORT;
        break;
      case 'q':
        rights[(int)piece_color::BLACK] |= LONG;
        break;
      case '-':
        break;
      default:
        throw std::invalid_argument("FEN: bad castling field " + castling);
    }
  }
  state->castlingrights[0] = static_cast<castling_rights>(rights[0]);
  state->castlingrights[1] = static_cast<castling_rights>(rights[1]);

  // 4. En passant target square, e.g. "e3". (0, 0) means none.
  if (ep != "-") {
    if (ep.size() != 2 || ep[0] < 'a' || ep[0] > 'h' || ep[1] < '1' ||
        ep[1] > '8') {
      throw std::invalid_argument("FEN: bad en passant square " + ep);
    }
    state->ep_square = std::make_tuple(ep[0] - 'a', ep[1] - '1');
  }

  // 5./6. The engine counts half moves from the start of the game.
  state->num_halfmoves =
      (fullmove - 1) * 2 + (state->to_move == piece_color::BLACK ? 1 : 0);
  (void)halfmove_clock;  // the 50-move rule is not implemented yet

  this->gh = state;
  this->legal_moves_history.clear();
  // Generate the legal moves right away: this also sets is_check and result
  // (checkmate or stalemate) for the new position.
  move_generation mg;
  this->legal_moves = mg.generate_all_moves(this->gh.get());
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

void engine::perform_move(move& m) { apply_move(find_legal_move(m), piece()); }

void engine::perform_move(move& m, piece& p) {
  apply_move(find_legal_move(m), p);
}

move engine::find_legal_move(const move& m) {
  ensure_legal_moves();
  for (const move& legal : this->legal_moves) {
    if (legal.from_x == m.from_x && legal.from_y == m.from_y &&
        legal.to_x == m.to_x && legal.to_y == m.to_y) {
      // Return the engine's own move, not the caller's: mover and type come
      // from the move generator, so a caller with wrong data can do no harm.
      return legal;
    }
  }
  throw std::invalid_argument("illegal move: " + to_uci(m));
}

void engine::apply_move(move m, piece promotion) {
  // m comes from find_legal_move(), so it is legal. Only the promotion piece
  // still has to be checked.
  piece mover = m.mover;
  if (is_promotion(m)) {
    piece_type t = promotion.get_piece_type();
    if (!promotion.is_valid() ||
        promotion.get_piece_color() != mover.get_piece_color() ||
        (t != QUEEN && t != ROOK && t != BISHOP && t != KNIGHT)) {
      throw std::invalid_argument(
          "promotion needs a queen, rook, bishop or "
          "knight of the moving side: " +
          to_uci(m));
    }
  } else if (promotion.is_valid()) {
    throw std::invalid_argument("not a promotion move: " + to_uci(m));
  }

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
    new_gm->curr_board.add_piece(promotion, to_x, to_y);
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
