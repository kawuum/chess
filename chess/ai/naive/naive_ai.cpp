#include "naive_ai.hpp"

#include <utility>
#include <vector>

#include "../../engine/evaluation.hpp"

naive_ai::naive_ai(engine* e, std::string name)
    : ai_template(e, std::move(name)) {}

move naive_ai::next_move() {
  game_history gh = eng->get_current_gamestate();
  std::vector<move> moves = eng->get_all_legal_moves(&gh);
  if (moves.empty()) return move{};  // all fields zero: type is NO_MOVE

  // TODO(naive_ai): choose the best move instead of the first one.
  //
  // Useful engine functions:
  // - eng->get_color_to_move(): the AI's own color (read it before trying
  //   moves, afterwards the other side is to move).
  // - eng->perform_move(m): plays m on the engine.
  // - eng->get_current_board(): the board after that move.
  // - eng->get_current_gamestate().result: WHITE_WIN, BLACK_WIN or DRAW if
  //   the move ended the game (checkmate or stalemate).
  // - eng->undo_move(): takes the move back.
  //
  // Every tried move has to be taken back before next_move() returns: the
  // engine is the one the GUI shows.
  return moves.front();
}

int naive_ai::evaluate([[maybe_unused]] const board& b,
                       [[maybe_unused]] piece_color me) const {
  // TODO(naive_ai): the evaluation, see the idea in naive_ai.hpp. Remove the
  // [[maybe_unused]] once b and me are used.
  return 0;
}
