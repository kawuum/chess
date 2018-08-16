#include "engine.hpp"
#include "move_generation.hpp"
#include "move_generation.hpp"

void engine::new_game() {
  this->gh = std::make_shared<game_history>();
  this->gh->num_halfmoves = 0;
  this->gh->to_move = piece_color::WHITE;
  this->gh->castlingrights[(int) piece_color::WHITE] = BOTH;
  this->gh->castlingrights[(int) piece_color::BLACK] = BOTH;
  this->gh->curr_board.add_piece(piece(piece_type::ROOK, piece_color::WHITE), 0, 0);
  this->gh->curr_board.add_piece(piece(piece_type::KNIGHT, piece_color::WHITE), 1, 0);
  this->gh->curr_board.add_piece(piece(piece_type::BISHOP, piece_color::WHITE), 2, 0);
  this->gh->curr_board.add_piece(piece(piece_type::QUEEN, piece_color::WHITE), 3, 0);
  this->gh->curr_board.add_piece(piece(piece_type::KING, piece_color::WHITE), 4, 0);
  this->gh->curr_board.add_piece(piece(piece_type::BISHOP, piece_color::WHITE), 5, 0);
  this->gh->curr_board.add_piece(piece(piece_type::KNIGHT, piece_color::WHITE), 6, 0);
  this->gh->curr_board.add_piece(piece(piece_type::ROOK, piece_color::WHITE), 7, 0);
  this->gh->curr_board.add_piece(piece(piece_type::PAWN, piece_color::WHITE), 0, 1);
  this->gh->curr_board.add_piece(piece(piece_type::PAWN, piece_color::WHITE), 1, 1);
  this->gh->curr_board.add_piece(piece(piece_type::PAWN, piece_color::WHITE), 2, 1);
  this->gh->curr_board.add_piece(piece(piece_type::PAWN, piece_color::WHITE), 3, 1);
  this->gh->curr_board.add_piece(piece(piece_type::PAWN, piece_color::WHITE), 4, 1);
  this->gh->curr_board.add_piece(piece(piece_type::PAWN, piece_color::WHITE), 5, 1);
  this->gh->curr_board.add_piece(piece(piece_type::PAWN, piece_color::WHITE), 6, 1);
  this->gh->curr_board.add_piece(piece(piece_type::PAWN, piece_color::WHITE), 7, 1);

  this->gh->curr_board.add_piece(piece(piece_type::ROOK, piece_color::BLACK), 0, 7);
  this->gh->curr_board.add_piece(piece(piece_type::KNIGHT, piece_color::BLACK), 1, 7);
  this->gh->curr_board.add_piece(piece(piece_type::BISHOP, piece_color::BLACK), 2, 7);
  this->gh->curr_board.add_piece(piece(piece_type::QUEEN, piece_color::BLACK), 3, 7);
  this->gh->curr_board.add_piece(piece(piece_type::KING, piece_color::BLACK), 4, 7);
  this->gh->curr_board.add_piece(piece(piece_type::BISHOP, piece_color::BLACK), 5, 7);
  this->gh->curr_board.add_piece(piece(piece_type::KNIGHT, piece_color::BLACK), 6, 7);
  this->gh->curr_board.add_piece(piece(piece_type::ROOK, piece_color::BLACK), 7, 7);
  this->gh->curr_board.add_piece(piece(piece_type::PAWN, piece_color::BLACK), 0, 6);
  this->gh->curr_board.add_piece(piece(piece_type::PAWN, piece_color::BLACK), 1, 6);
  this->gh->curr_board.add_piece(piece(piece_type::PAWN, piece_color::BLACK), 2, 6);
  this->gh->curr_board.add_piece(piece(piece_type::PAWN, piece_color::BLACK), 3, 6);
  this->gh->curr_board.add_piece(piece(piece_type::PAWN, piece_color::BLACK), 4, 6);
  this->gh->curr_board.add_piece(piece(piece_type::PAWN, piece_color::BLACK), 5, 6);
  this->gh->curr_board.add_piece(piece(piece_type::PAWN, piece_color::BLACK), 6, 6);
  this->gh->curr_board.add_piece(piece(piece_type::PAWN, piece_color::BLACK), 7, 6);
}

void engine::perform_move(uint8_t from_x, uint8_t from_y, uint8_t to_x, uint8_t to_y) {
  // FIXME GUESSING MOVE_TYPE HERE, BAD IDEA!!
  piece p = this->gh->curr_board.get_piece(from_x, from_y); 
  this->perform_move(p, from_x, from_y, to_x, to_y, move_type::MOVE);
}

std::vector<move> engine::get_legal_moves(piece& p, uint8_t from_x, uint8_t from_y) {
  move_generation mg;
  return mg.generate_moves(p, from_x, from_y, this->gh);
}

void engine::perform_move(move& m, piece& p) {
  std::cout << "...right method called..." << std::endl;
  perform_move(m.mover, m.from_x, m.from_y, m.to_x, m.to_y, m.type, p);
}


void engine::perform_move(piece& p,
                          uint8_t from_x,
                          uint8_t from_y,
                          uint8_t to_x,
                          uint8_t to_y,
                          move_type mt,
                          piece promotion) {
  // TODO: CHECK IF MOVE IS LEGAL
  // TODO: Possible memory leaks...

  move_generation mg;
  std::vector<move> moves = mg.generate_moves(p, from_x, from_y, this->gh);

  //check if move to perform is in generated moves --> LEGALITY CHECK

  //if(!p.is_valid())
  piece pie = this->gh->curr_board.get_piece(from_x, from_y);
  if (!pie.is_valid())
    return;
  if (pie.get_piece_color() != this->gh->to_move)
    return;
  /*if(pie != p)
      return; //should we just return here? or maybe ignore if the move seems to be legal?
  */

  std::shared_ptr<game_history> new_gm = std::make_shared<game_history>();

  if (pie.get_piece_type() == PAWN && (std::abs(from_y - to_y) == 2)) {
    // A pawn is getting moved. Since this is a double jump, we need to add an en passant square.
    new_gm->ep_square = std::make_tuple(to_x, (pie.get_piece_color() == piece_color::WHITE ? to_y - 1 : to_y + 1));
  }

  new_gm->curr_board = this->gh->curr_board;
  new_gm->curr_board.move_piece(pie, from_x, from_y, to_x, to_y);
  if (mt == CASTLING) {
    //also move rook
    if (from_x > to_x) {
      // long castle
      new_gm->curr_board.move_piece(piece(ROOK, pie.get_piece_color()), 0, from_y, 3, to_y);
    } else {
      // short castle
      new_gm->curr_board.move_piece(piece(ROOK, pie.get_piece_color()), 7, from_y, 5, to_y);
    }
  } else if (mt == PROMOTION || mt == CAPTURING_PROMOTION) {
    std::cout << "...right if clause caught..." << std::endl;
    new_gm->curr_board.remove_piece(p, to_x, to_y);
    new_gm->curr_board.add_piece(promotion, to_x, to_y);
  }
  piece captured_piece = this->gh->curr_board.get_piece(to_x, to_y);
  if (captured_piece.is_valid() && mt != ENPASSANT) {
    new_gm->curr_board.remove_piece(captured_piece, to_x, to_y);
  } else if (mt == ENPASSANT) {
    piece ep_piece = this->gh->curr_board.get_piece(std::get<0>(this->gh->ep_square), from_y);
    new_gm->curr_board.remove_piece(ep_piece, std::get<0>(this->gh->ep_square), from_y);
  }
  this->gh->performed_move = (move) {p, from_x, from_y, to_x, to_y, mt};
  new_gm->num_halfmoves = this->gh->num_halfmoves + 1;
  new_gm->castlingrights[0] = this->gh->castlingrights[0];
  new_gm->castlingrights[1] = this->gh->castlingrights[1];
  castling_rights cr = this->gh->castlingrights[(int) this->gh->to_move];
  // handling castling privileges
  if (pie.get_piece_type() == KING) {
    new_gm->castlingrights[(int) this->gh->to_move] = NONE;
  }
  if ((pie.get_piece_type() == ROOK && cr != NONE) || (captured_piece.get_piece_type() == ROOK
      && this->gh->castlingrights[(int) captured_piece.get_piece_color()] != NONE)) {
    uint8_t starting_y = this->gh->to_move == piece_color::WHITE ? 0 : 7;
    if (from_y == starting_y) {
      if (from_x == 0 && (cr == BOTH || cr == LONG)) {
        // losing long castling privilege
        cr = (castling_rights) (cr - LONG);
      } else if (from_x == 7 && (cr == BOTH || cr == SHORT)) {
        // losing short castling privilege
        cr = (castling_rights) (cr - SHORT);
      }
      new_gm->castlingrights[(int) this->gh->to_move] = cr;
    } else if (to_y == starting_y) {
      // A rook was captured, we can eliminate a castling privilege here
      cr = this->gh->castlingrights[(int) captured_piece.get_piece_color()];
      if (to_x == 0 && (cr == BOTH || cr == LONG)) {
        // losing long castling privilege
        cr = (castling_rights) (cr - LONG);
      } else if (to_x == 7 && (cr == BOTH || cr == SHORT)) {
        // losing short castling privilege
        cr = (castling_rights) (cr - SHORT);
      }
      new_gm->castlingrights[(int) captured_piece.get_piece_color()] = cr;
    }
  }
  new_gm->to_move = this->gh->to_move == piece_color::WHITE ? piece_color::BLACK : piece_color::WHITE;
  new_gm->prev = this->gh;

  this->gh = new_gm;
}

void engine::perform_move(move& m) {
  perform_move(m.mover, m.from_x, m.from_y, m.to_x, m.to_y, m.type);
}

board &engine::get_current_board() {
  return this->gh->curr_board;
}

void engine::undo_move() {
  if (this->gh->prev != NULL)
    this->gh = this->gh->prev;
}

piece_color engine::get_color_to_move() {
  return this->gh->to_move;
}

game_history engine::get_current_gamestate() {
  game_history g = *this->gh;
  return g;
}


