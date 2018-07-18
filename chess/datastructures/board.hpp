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

  board() : num_moves_performed(0) {
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
  void movePiece(uint8_t from_x, uint8_t from_y, uint8_t to_x, uint8_t to_y);

  /**
   * Move piece p on form_coordinates to to_coordinates.
   * Does not perform any check if the move is allowed, except:
   * If there is no piece p at from_coordinates, this method will throw an error.
   * If there is already an piece at to_coordinates, this method might throw an error.
   * @param from_x
   * @param from_y
   * @param to_x
   * @param to_y
   */
  void movePiece(piece p, uint8_t from_x, uint8_t from_y, uint8_t to_x, uint8_t to_y);

  /**
   * Move piece p_from on form_coordinates to to_coordinates and remove piece p_to from the board.
   * Does not perform any check if the move is allowed, except:
   * If there is no piece p_from at from_coordinates, this method will throw an error.
   * If there is no piece p_to at to_coordinates, this method will throw an error.
   * If there is already an piece other than p_to at to_coordinates, this method might throw an error.
   * @param from_x
   * @param from_y
   * @param to_x
   * @param to_y
   */
  void movePiece(piece p_from, uint8_t from_x, uint8_t from_y, piece p_to, uint8_t to_x, uint8_t to_y);

  /**
   * Removes whatever piece is found on coordinates from the board.
   * Does not perform any check if the move is allowed, except:
   * If there is no piece at the coordinates, this method will throw an error.
   * @param from_x
   * @param from_y
   * @param to_x
   * @param to_y
   */
  void removePiece(uint8_t from_x, uint8_t from_y, uint8_t to_x, uint8_t to_y);

  /**
   * Removes piece p on coordinates from the board.
   * Does not perform any check if the move is allowed, except:
   * If there is no piece p at the coordinates, this method will throw an error.
   * @param from_x
   * @param from_y
   * @param to_x
   * @param to_y
   */
  void removePiece(piece p, uint8_t from_x, uint8_t from_y, uint8_t to_x, uint8_t to_y);

  /**
   * Adds the specified piece p to the specified positions.
   * If there is already a piece at the given coordinates, this method might throw an error.
   * @param p
   * @param x
   * @param y
   */
  void addPiece(piece p, uint8_t x, uint8_t y);

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

 private:

  // White pieces
  bit_matrix white_king;
  bit_matrix white_queen;
  bit_matrix white_bishops;
  bit_matrix white_knights;
  bit_matrix white_pawns;

  // Black pieces
  bit_matrix black_king;
  bit_matrix black_queen;
  bit_matrix black_bishops;
  bit_matrix black_knights;
  bit_matrix black_pawns;

  uint64_t num_halfmoves_performed;

};

#endif //CHESS_BOARD_HPP
