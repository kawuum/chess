#include "board.hpp"

#include <stdexcept>

// Every method that changes the board updates both representations: the
// bitboard of the piece and the byte in squares.

void board::add_piece(piece p, uint8_t x, uint8_t y) {
  bitboard_for(p).set_bit_at(x, y);
  squares[8 * y + x] = code_of(p);
}

void board::remove_piece(piece p, uint8_t from_x, uint8_t from_y) {
  bitboard_for(p).unset_bit_at(from_x, from_y);
  // Only clear the square if this piece is the one recorded there. The engine
  // moves the capturing piece onto the target square first and removes the
  // captured piece afterwards; the capturing piece has to stay.
  uint8_t& square = squares[8 * from_y + from_x];
  if (square == code_of(p)) square = 0;
}

void board::remove_piece(uint8_t from_x, uint8_t from_y) {
  piece p = get_piece(from_x, from_y);
  if (p.is_valid()) remove_piece(p, from_x, from_y);
}

void board::move_piece(piece p, uint8_t from_x, uint8_t from_y, uint8_t to_x,
                       uint8_t to_y) {
  remove_piece(p, from_x, from_y);
  add_piece(p, to_x, to_y);
}

void board::move_piece(piece p_from, uint8_t from_x, uint8_t from_y, piece p_to,
                       uint8_t to_x, uint8_t to_y) {
  remove_piece(p_to, to_x, to_y);
  move_piece(p_from, from_x, from_y, to_x, to_y);
}

void board::move_piece(uint8_t from_x, uint8_t from_y, uint8_t to_x,
                       uint8_t to_y) {
  piece p = get_piece(from_x, from_y);
  if (p.is_valid()) move_piece(p, from_x, from_y, to_x, to_y);
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
  for (uint8_t& square : squares) square = 0;
}

bool board::check_sanity() {
  // 1. Every square holds at most one piece in the bitboards, and squares
  //    names exactly that piece (or nothing).
  for (uint8_t y = 0; y < 8; ++y) {
    for (uint8_t x = 0; x < 8; ++x) {
      int found = 0;
      uint8_t expected = 0;
      for (int code = 1; code <= 12; ++code) {
        if (get_bit_matrix_for_piece(kDecode[code]).get_bit_at(x, y)) {
          ++found;
          expected = static_cast<uint8_t>(code);
        }
      }
      if (found > 1 || squares[8 * y + x] != expected) return false;
    }
  }
  // 2. No square is used by both colors (follows from 1, kept as a cheap
  //    second check).
  return (get_bit_matrix_for_color(piece_color::WHITE) &
          get_bit_matrix_for_color(piece_color::BLACK)) == 0;
}

bool board::is_empty() { return this->get_num_pieces() == 0; }

uint8_t board::get_num_pieces() {
  return this->get_num_pieces(piece_color::BLACK) +
         this->get_num_pieces(piece_color::WHITE);
}

uint8_t board::get_num_pieces(piece_color c) {
  return get_bit_matrix_for_color(c).get_num_bits_set();
}

uint8_t board::get_num_pieces(piece p) {
  return this->get_bit_matrix_for_piece(p).get_num_bits_set();
}

uint8_t board::get_num_pieces(piece_type t) {
  return this->get_num_pieces(piece(t, piece_color::WHITE)) +
         this->get_num_pieces(piece(t, piece_color::BLACK));
}

bit_matrix board::get_bit_matrix_for_color(piece_color c) const {
  return (get_bit_matrix_for_piece(piece(BISHOP, c)) |
          get_bit_matrix_for_piece(piece(KING, c)) |
          get_bit_matrix_for_piece(piece(KNIGHT, c)) |
          get_bit_matrix_for_piece(piece(PAWN, c)) |
          get_bit_matrix_for_piece(piece(QUEEN, c)) |
          get_bit_matrix_for_piece(piece(ROOK, c)));
}

const bit_matrix& board::get_bit_matrix_for_piece(piece p) const {
  if (p.is_valid()) {
    bool white = p.get_piece_color() == piece_color::WHITE;
    switch (p.get_piece_type()) {
      case KING:
        return white ? white_king : black_king;
      case QUEEN:
        return white ? white_queens : black_queens;
      case BISHOP:
        return white ? white_bishops : black_bishops;
      case KNIGHT:
        return white ? white_knights : black_knights;
      case ROOK:
        return white ? white_rooks : black_rooks;
      case PAWN:
        return white ? white_pawns : black_pawns;
    }
  }
  // A function returning a reference must not just fall off its end
  // (undefined behavior), so fail loudly instead.
  throw std::invalid_argument("get_bit_matrix_for_piece: invalid piece");
}

bit_matrix& board::bitboard_for(piece p) {
  // Same lookup as the const version; the const_cast is safe because this
  // object itself is not const.
  return const_cast<bit_matrix&>(get_bit_matrix_for_piece(p));
}
