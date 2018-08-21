//
// Created by Siegfried Depner on 7/17/18.
//

#include "board.hpp"

void board::add_piece(piece p, uint8_t x, uint8_t y) {
  switch (p.get_piece_type()) {
    case BISHOP:
      if (p.is_white()) this->white_bishops.set_bit_at(x, y);
      else this->black_bishops.set_bit_at(x, y);
      break;
    case KING:
      if (p.is_white()) this->white_king.set_bit_at(x, y);
      else this->black_king.set_bit_at(x, y);
      break;
    case KNIGHT:
      if (p.is_white()) this->white_knights.set_bit_at(x, y);
      else this->black_knights.set_bit_at(x, y);
      break;
    case PAWN:
      if (p.is_white()) this->white_pawns.set_bit_at(x, y);
      else this->black_pawns.set_bit_at(x, y);
      break;
    case QUEEN:
      if (p.is_white()) this->white_queens.set_bit_at(x, y);
      else this->black_queens.set_bit_at(x, y);
      break;
    case ROOK:
      if (p.is_white()) this->white_rooks.set_bit_at(x, y);
      else this->black_rooks.set_bit_at(x, y);
      break;
  }
}

piece board::get_piece(uint8_t x, uint8_t y) {
  if (this->black_bishops.get_bit_at(x, y)) {
    return piece(piece_type::BISHOP, piece_color::BLACK);
  } else if (this->black_king.get_bit_at(x, y)) {
    return piece(piece_type::KING, piece_color::BLACK);
  } else if (this->black_knights.get_bit_at(x, y)) {
    return piece(piece_type::KNIGHT, piece_color::BLACK);
  } else if (this->black_pawns.get_bit_at(x, y)) {
    return piece(piece_type::PAWN, piece_color::BLACK);
  } else if (this->black_queens.get_bit_at(x, y)) {
    return piece(piece_type::QUEEN, piece_color::BLACK);
  } else if (this->black_rooks.get_bit_at(x, y)) {
    return piece(piece_type::ROOK, piece_color::BLACK);
  } else if (this->white_bishops.get_bit_at(x, y)) {
    return piece(piece_type::BISHOP, piece_color::WHITE);
  } else if (this->white_king.get_bit_at(x, y)) {
    return piece(piece_type::KING, piece_color::WHITE);
  } else if (this->white_knights.get_bit_at(x, y)) {
    return piece(piece_type::KNIGHT, piece_color::WHITE);
  } else if (this->white_pawns.get_bit_at(x, y)) {
    return piece(piece_type::PAWN, piece_color::WHITE);
  } else if (this->white_queens.get_bit_at(x, y)) {
    return piece(piece_type::QUEEN, piece_color::WHITE);
  } else if (this->white_rooks.get_bit_at(x, y)) {
    return piece(piece_type::ROOK, piece_color::WHITE);
  }
  // If there is no piece, return an invalid one.
  return piece();
}

void board::clear() {
  this->black_bishops.clear();
  this->black_king.clear();
  this->black_knights.clear();
  this->black_pawns.clear();
  this->black_queens.clear();
  this->black_rooks.clear();
  this->white_bishops.clear();
  this->white_king.clear();
  this->white_knights.clear();
  this->white_pawns.clear();
  this->white_queens.clear();
  this->white_rooks.clear();
}

bool board::check_sanity() {
  if ((white_bishops & white_king) != 0
      || (white_bishops & white_knights) != 0
      || (white_bishops & white_pawns) != 0
      || (white_bishops & white_queens) != 0
      || (white_bishops & white_rooks) != 0
      || (white_king & white_knights) != 0
      || (white_king & white_pawns) != 0
      || (white_king & white_queens) != 0
      || (white_king & white_rooks) != 0
      || (white_knights & white_pawns) != 0
      || (white_knights & white_queens) != 0
      || (white_knights & white_rooks) != 0
      || (white_pawns & white_queens) != 0
      || (white_pawns & white_rooks) != 0
      || (white_queens & white_rooks) != 0) {
    return false;
  }

  if ((black_bishops & black_king) != 0
      || (black_bishops & black_knights) != 0
      || (black_bishops & black_pawns) != 0
      || (black_bishops & black_queens) != 0
      || (black_bishops & black_rooks) != 0
      || (black_king & black_knights) != 0
      || (black_king & black_pawns) != 0
      || (black_king & black_queens) != 0
      || (black_king & black_rooks) != 0
      || (black_knights & black_pawns) != 0
      || (black_knights & black_queens) != 0
      || (black_knights & black_rooks) != 0
      || (black_pawns & black_queens) != 0
      || (black_pawns & black_rooks) != 0
      || (black_queens & black_rooks) != 0) {
    return false;
  }

  if ((this->get_bit_matrix_for_color(piece_color::WHITE)
      & this->get_bit_matrix_for_color(piece_color::BLACK)).get_raw() != 0) {
    return false;
  }

  return true;
}

bool board::is_empty() {
  return this->get_num_pieces() == 0;
}


uint8_t board::get_num_pieces() {
  return this->get_num_pieces(piece_color::BLACK) + this->get_num_pieces(piece_color::WHITE);
}

uint8_t board::get_num_pieces(piece_color c) {
  return this->get_num_pieces(piece(BISHOP, c)) +
      this->get_num_pieces(piece(KING, c)) +
      this->get_num_pieces(piece(KNIGHT, c)) +
      this->get_num_pieces(piece(PAWN, c)) +
      this->get_num_pieces(piece(QUEEN, c)) +
      this->get_num_pieces(piece(ROOK, c));
}

uint8_t board::get_num_pieces(piece p) {
  return this->get_bit_matrix_for_piece(p).get_num_bits_set();
}

uint8_t board::get_num_pieces(piece_type t) {
  return this->get_num_pieces(piece(t, piece_color::WHITE)) + this->get_num_pieces(piece(t, piece_color::BLACK));
}

bit_matrix board::get_bit_matrix_for_color(piece_color c) {
  return (this->get_bit_matrix_for_piece(piece(BISHOP, c))
      | this->get_bit_matrix_for_piece(piece(KING, c))
      | this->get_bit_matrix_for_piece(piece(KNIGHT, c))
      | this->get_bit_matrix_for_piece(piece(PAWN, c))
      | this->get_bit_matrix_for_piece(piece(QUEEN, c))
      | this->get_bit_matrix_for_piece(piece(ROOK, c)));
}

bit_matrix &board::get_bit_matrix_for_piece(piece p) {
  switch (p.get_piece_color()) {
    case piece_color::WHITE: {
      switch (p.get_piece_type()) {
        case BISHOP:
          return this->white_bishops;
          break;
        case KING:
          return this->white_king;
          break;
        case KNIGHT:
          return this->white_knights;
          break;
        case PAWN:
          return this->white_pawns;
          break;
        case QUEEN:
          return this->white_queens;
          break;
        case ROOK:
          return this->white_rooks;
          break;
      }
    }
      break;
    case piece_color::BLACK: {
      switch (p.get_piece_type()) {
        case BISHOP:
          return this->black_bishops;
          break;
        case KING:
          return this->black_king;
          break;
        case KNIGHT:
          return this->black_knights;
          break;
        case PAWN:
          return this->black_pawns;
          break;
        case QUEEN:
          return this->black_queens;
          break;
        case ROOK:
          return this->black_rooks;
          break;
      }
    }
      break;
  }
}

void board::move_piece(piece p, uint8_t from_x, uint8_t from_y, uint8_t to_x, uint8_t to_y) {
  this->remove_piece(p, from_x, from_y);
  this->get_bit_matrix_for_piece(p).set_bit_at(to_x, to_y);
}

void board::move_piece(piece p_from, uint8_t from_x, uint8_t from_y, piece p_to, uint8_t to_x, uint8_t to_y) {
  this->remove_piece(p_to, to_x, to_y);
  this->move_piece(p_from, from_x, from_y, to_x, to_y);
}

void board::move_piece(uint8_t from_x, uint8_t from_y, uint8_t to_x, uint8_t to_y) {
  bit_matrix *rightOne = this->find_piece(from_x, from_y);
  rightOne->unset_bit_at(from_x, from_y);
  rightOne->set_bit_at(to_x, to_y);
}

void board::remove_piece(piece p, uint8_t from_x, uint8_t from_y) {
  this->get_bit_matrix_for_piece(p).unset_bit_at(from_x, from_y);
}

void board::remove_piece(uint8_t from_x, uint8_t from_y) {
  this->find_piece(from_x, from_y)->unset_bit_at(from_x, from_y);
}

bit_matrix *board::find_piece(uint8_t from_x, uint8_t from_y) {
  if (this->white_bishops.get_bit_at(from_x, from_y)) return &white_bishops;
  else if (this->white_king.get_bit_at(from_x, from_y)) return &white_king;
  else if (this->white_knights.get_bit_at(from_x, from_y)) return &white_knights;
  else if (this->white_pawns.get_bit_at(from_x, from_y)) return &white_pawns;
  else if (this->white_queens.get_bit_at(from_x, from_y)) return &white_queens;
  else if (this->white_rooks.get_bit_at(from_x, from_y)) return &white_rooks;
  else if (this->black_bishops.get_bit_at(from_x, from_y)) return &black_bishops;
  else if (this->black_king.get_bit_at(from_x, from_y)) return &black_king;
  else if (this->black_knights.get_bit_at(from_x, from_y)) return &black_knights;
  else if (this->black_pawns.get_bit_at(from_x, from_y)) return &black_pawns;
  else if (this->black_queens.get_bit_at(from_x, from_y)) return &black_queens;
  else if (this->black_rooks.get_bit_at(from_x, from_y)) return &black_rooks;
  else return NULL; // silence compiler warning
}
