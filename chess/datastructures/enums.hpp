#ifndef CHESS_ENUMS_HPP
#define CHESS_ENUMS_HPP

#include <stdint.h>

// All enums are stored in one byte (": uint8_t") instead of the default four
// (an int): piece and move contain them and are copied very often.

enum piece_type : uint8_t { KING, QUEEN, BISHOP, KNIGHT, ROOK, PAWN };
enum class piece_color : uint8_t { WHITE, BLACK };

enum castling_rights : uint8_t { NONE = 0, SHORT = 1, LONG = 2, BOTH = 3 };

enum game_result : uint8_t { RUNNING, DRAW, WHITE_WIN, BLACK_WIN };

// Why a game ended in a draw (game_result DRAW).
enum draw_reason : uint8_t {
  NO_DRAW,
  STALEMATE,              // the side to move has no legal move, not in check
  THREEFOLD_REPETITION,   // the same position for the third time
  FIFTY_MOVE_RULE,        // 50 moves of each side without capture or pawn move
  INSUFFICIENT_MATERIAL,  // no side can checkmate any more
};

#endif  // CHESS_ENUMS_HPP
