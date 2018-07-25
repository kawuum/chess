#include <stdint.h>
#include "piece.hpp"
#include "bit_matrix.hpp"
//
// Created by Siegfried Depner on 7/17/18.
//

#ifndef CHESS_BOARD_HPP
#define CHESS_BOARD_HPP

// TODO: Implement all methods + fix comments accordingly
// TODO: Implement move / copy constructors carefully :-)


/**
 * This class implements a chess board.
 *
 * It should not contain logic such as how to move different pieces according to games rules. Instead it is
 * a data structure with some simple Operations on it (such as move an arbitrary piece from coordinate A to coordinate B
 * without checking if this move is permitted given any game rules).
 *
 * This class is :-) implemented with performance in mind. This implies, that as little checks as possible should
 * be performed when calling a manipulation method, causing that methods in general give very few guarantees for
 * proper error handling (which is specified in the comment for each method, if for example a method "might throw
 * an error", it only throws it when discovering the error during the manipulation step, but not when it would require
 * additional steps to detect an error). If you want to check the sanity of the data structure, you can use the
 * check_sanity method which should throw appropriate errors.
 */

class board {

 public:

  board() {
  }

  /**
   * Move whatever piece is found on form_coordinates to to_coordinates.
   * Does not perform any check if the move is allowed, except:
   * If there is no piece at from_coordinates, this methods will throw an error.
   * If there is already an piece at to_coordinates, this methods might throw an error.
   * @param from_x
   * @param from_y
   * @param to_x
   * @param to_y
   */
  void move_piece(uint8_t from_x, uint8_t from_y, uint8_t to_x, uint8_t to_y);

  /**
   * Move piece p on form_coordinates to to_coordinates.
   * Does not perform any check if the move is allowed, except:
   * If there is no piece p at from_coordinates, this method will throw an error.
   * If there is already an piece at to_coordinates, this method might throw an error.
   * @param p
   * @param from_x
   * @param from_y
   * @param to_x
   * @param to_y
   */
  void move_piece(piece p, uint8_t from_x, uint8_t from_y, uint8_t to_x, uint8_t to_y);

  /**
   * Move piece p_from on form_coordinates to to_coordinates and remove piece p_to from the board.
   * Does not perform any check if the move is allowed, except:
   * If there is no piece p_from at from_coordinates, this method will throw an error.
   * If there is no piece p_to at to_coordinates, this method will throw an error.
   * If there is already an piece other than p_to at to_coordinates, this method might throw an error.
   * @param p_from
   * @param from_x
   * @param from_y
   * @param p_to
   * @param to_x
   * @param to_y
   */
  void move_piece(piece p_from, uint8_t from_x, uint8_t from_y, piece p_to, uint8_t to_x, uint8_t to_y);

  /**
   * Removes whatever piece is found on coordinates from the board.
   * Does not perform any check if the remove is allowed, except:
   * If there is no piece at the coordinates, this method will throw an error.
   * @param from_x
   * @param from_y
   */
  void remove_piece(uint8_t from_x, uint8_t from_y);

  /**
   * Removes piece p on coordinates from the board.
   * Does not perform any check if the move is allowed, except:
   * If there is no piece p at the coordinates, this method will throw an error.
   * @param p
   * @param from_x
   * @param from_y
   */
  void remove_piece(piece p, uint8_t from_x, uint8_t from_y);

  /**
   * Adds the specified piece p to the specified positions.
   * If there is already a piece at the given coordinates, this method might throw an error.
   * @param p
   * @param x
   * @param y
   */
  void add_piece(piece p, uint8_t x, uint8_t y);

  /**
   * Removes all pieces from the board.
   */
  void clear();

  /**
   * This method checks the board for sanity. It does NOT check any game-specific sanity (if there are such), instead
   * it only checks if the internal representation is sane.
   * List of states which are not considered to be sane (to be continued...):
   * * Multiple pieces on the same coordinates
   * @return true if the board is sane, false otherwise.
   */
  bool check_sanity();

  /**
   * Allows for low-level access of the bit_matrices.
   * @param p The pieces, the bit_matrix should be returned for.
   * @return Reference to the respective bit_matrix.
   */
  bit_matrix &get_bit_matrix_for_piece(piece p);

  /**
   * Assembles the bit_matrices for the respective colors.
   * @param c The color the bit_matrix should be returned for.
   * @return The respective bit_matrix.
   */
  bit_matrix get_bit_matrix_for_color(piece_color c);

  /**
   * Returns true if no piece is stored on this board, false otherwise.
   * @return true if no piece is stored on this board, false otherwise.
   */
  bool is_empty();

  /**
   * Returns the total number of pieces on this board.
   * @return Total number of pieces.
   */
  uint8_t get_num_pieces();

  /**
   * Returns the total number of pieces with the given piece_type on this board.
   * @param t The piece_type.
   * @return Total number of pieces of the given type.
   */
  uint8_t get_num_pieces(piece_type t);

  /**
   * Returns the total number of pieces with the given piece_color on this board.
   * @param c The piece_color.
   * @return Total number of pieces with the given piece_color.
   */
  uint8_t get_num_pieces(piece_color c);

  /**
  * Returns the total number of pieces.
  * @param t The piece.
  * @return Total number of pieces.
  */
  uint8_t get_num_pieces(piece p);

 private:
  bit_matrix *find_piece(uint8_t from_x, uint8_t from_y);

  // White pieces
  bit_matrix white_king;
  bit_matrix white_queens;
  bit_matrix white_bishops;
  bit_matrix white_knights;
  bit_matrix white_pawns;
  bit_matrix white_rooks;

  // Black pieces
  bit_matrix black_king;
  bit_matrix black_queens;
  bit_matrix black_bishops;
  bit_matrix black_knights;
  bit_matrix black_pawns;
  bit_matrix black_rooks;
};

#endif //CHESS_BOARD_HPP
