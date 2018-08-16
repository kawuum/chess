#include "../datastructures/board.hpp"
#include "game_history.hpp"
#include "../datastructures/enums.hpp"
#include <memory>
#include <vector>
#include "move.hpp"

#ifndef ENGINE_HPP
#define ENGINE_HPP

class engine {
 public:
  engine() {}
  /**
   * Resets game state to start a new standard chess game.
   */
  void new_game();
  /**
   * Resets game state to start a new chess game given the according FEN string.
   * @param fen The Forsyth-Edwards-Notation of the game that should be started.
   */
  void new_game(std::string fen);
  /**
   * Performs a move given the move to be performed.
   * @param m The move to be performed
   */
  void perform_move(move m);
  /**
* Performs a promotion move given the move to be performed.
* @param m The move to be performed
* @param p The piece the pawn should be promoted to
*/
  void perform_move(move& m, piece& p);
  /**
   * Reverses the last move.
   */
  void undo_move();
  /**
   * Returns the color whose turn it is.
   * @returns The color of the player whose turn it is.
   */
  piece_color get_color_to_move();
  /**
   * Returns all legal moves for the given piece.
   * @param p The piece we want to find legal moves for
   * @param from_x x coordinate of the given piece
   * @param from_y y coordinate of the given piece
   * @returns A vector containing all legal moves for the given piece
   */
  std::vector<move> get_legal_moves(piece& p, uint8_t from_x, uint8_t from_y);
  /**
   * Returns a reference of the current board.
   * @returns A reference of the current board
   */
  board &get_current_board();
  /**
   * Returns a copy of the current game state, containing information like the current board, castling rights, number of halfmoves performed etc.
   * @returns A copy of the current game state
   */
  game_history get_current_gamestate();
 private:
  /**
   * Performs a move given only the originating and target coordinates.
   * @param from_x x coordinate of moving piece
   * @param from_y y coordinate of moving piece
   * @param to_x x coordinate of target square
   * @param to_y y coordinate of target square
   */
  void perform_move(uint8_t from_x, uint8_t from_y, uint8_t to_x, uint8_t to_y);
  /**
   * Performs a move given the moving piece, its originating coordinates and the coordinates of the target square.
   * @param p The piece to move
   * @param from_x x coordinate of moving piece
   * @param from_y y coordinate of moving piece
   * @param to_x x coordinate of target square
   * @param to_y y coordinate of target square
   * @param mt The type of move to be performed
   * @param promotion The piece a potential promoting pawn should promote to
   */
  void perform_move(piece p,
                    uint8_t from_x,
                    uint8_t from_y,
                    uint8_t to_x,
                    uint8_t to_y,
                    move_type mt,
                    piece& promotion = piece());
  std::shared_ptr<game_history> gh;
};

#endif
