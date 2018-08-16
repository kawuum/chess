//
// Created by Siegfried Depner on 7/12/18.
//

#ifndef CHESS_ENUMS_HPP
#define CHESS_ENUMS_HPP

enum piece_type {
  KING, QUEEN, BISHOP, KNIGHT, ROOK, PAWN
};
enum class piece_color {
  WHITE, BLACK
};

enum castling_rights {
  NONE = 0,
  SHORT = 1,
  LONG = 2,
  BOTH = 3
};

#endif //CHESS_ENUMS_HPP
