#pragma once

#include "../datastructures/board.hpp"
#include "../datastructures/enums.hpp"
#include "game_history.hpp"

/**
 * The draw rules apart from stalemate (stalemate is found by the move
 * generator: no legal move and not in check).
 *
 * Simplifications against the official rules (FIDE), as usual in chess
 * programs: threefold repetition and the 50-move rule end the game at once
 * (officially a player has to claim the draw), and the en passant square
 * counts as stored by the engine, i.e. after every double step of a pawn,
 * even if no en passant capture is possible. That can only make a
 * repetition count later, never too early.
 */
namespace draw_rules {

/**
 * True if no side can checkmate any more, whatever is played: king against
 * king, king and one bishop or knight against king, or only bishops that
 * all stand on squares of the same color. Two knights or knight against
 * knight are not a draw: a mate is still possible (if the other side
 * helps).
 */
bool insufficient_material(const board& b);

/**
 * How often the position of gh has occurred in the game, gh itself
 * included. The same position means: same board, same side to move, same
 * castling rights and same en passant square.
 */
int repetitions(const game_history& gh);

/**
 * The draw rule that ends the game in position gh, or NO_DRAW. Checked in
 * this order: insufficient material, 50-move rule (100 half moves without
 * capture or pawn move), threefold repetition.
 */
draw_reason applying_rule(const game_history& gh);

}  // namespace draw_rules
