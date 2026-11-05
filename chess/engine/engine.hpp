#include <memory>
#include <vector>

#include "../datastructures/board.hpp"
#include "../datastructures/enums.hpp"
#include "game_history.hpp"
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
   * Performs a move. Only the start and target square of m (and for
   * promotions the promotion piece) are used; the engine looks the move up in
   * its list of legal moves.
   * @param m The move to be performed, e.g. one from get_legal_moves()
   * @throws std::invalid_argument if the move is not legal, for example a
   * promotion to a king or pawn. The position is unchanged then.
   */
  void perform_move(const move& m);
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
   * Returns all legal moves of the piece on the given square.
   * @param from_x x coordinate of the piece
   * @param from_y y coordinate of the piece
   * @returns A vector containing all legal moves starting on that square
   * (empty if there is no piece of the side to move)
   */
  std::vector<move> get_legal_moves(uint8_t from_x, uint8_t from_y);
  /**
   * Returns all legal moves for the player whose turn it is.
   * @param gh A pointer to the current game_history
   * @returns A vector containing all legal moves for player
   */
  std::vector<move> get_all_legal_moves(game_history* gh);
  /**
   * Returns a reference of the current board.
   * @returns A reference of the current board
   */
  board& get_current_board();
  /**
   * Returns a copy of the current game state, containing information like the
   * current board, castling rights, number of halfmoves performed etc.
   * @returns A copy of the current game state
   */
  game_history get_current_gamestate();

 private:
  /** Fills legal_moves for the current position if it is empty. */
  void ensure_legal_moves();
  /**
   * Looks up m (by start and target square, for promotions also by the
   * promotion piece) in the legal moves.
   * @returns The engine's own copy of the move
   * @throws std::invalid_argument if there is no such legal move
   */
  move find_legal_move(const move& m);
  /**
   * Plays a legal move and creates the next game state.
   * @param m A move from the legal move list
   */
  void apply_move(move m);
  std::shared_ptr<game_history> gh;
  // Legal moves of the current position (cache, see ensure_legal_moves()).
  std::vector<move> legal_moves;
  // Legal moves of the earlier positions, one entry per move played. Lets
  // undo_move() restore the cache instead of generating the moves again.
  std::vector<std::vector<move>> legal_moves_history;
};

#endif
