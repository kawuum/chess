#ifndef CHESS_ENUMS_HPP
#define CHESS_ENUMS_HPP

#include <stdint.h>

// All enums are stored in one byte (": uint8_t") instead of the default four
// (an int): piece and move contain them and are copied very often.

enum piece_type : uint8_t { KING, QUEEN, BISHOP, KNIGHT, ROOK, PAWN };
enum class piece_color : uint8_t { WHITE, BLACK };

enum castling_rights : uint8_t { NONE = 0, SHORT = 1, LONG = 2, BOTH = 3 };

enum game_result : uint8_t { RUNNING, DRAW, WHITE_WIN, BLACK_WIN };

#endif  // CHESS_ENUMS_HPP
