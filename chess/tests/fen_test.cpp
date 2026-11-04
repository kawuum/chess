// Tests for engine::new_game(fen): parsing of all FEN fields, error handling
// and the game state of loaded positions.
#include <stdexcept>

#include "test_helpers.hpp"

namespace {

const char* const kStart =
    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

TEST(Fen, StartPositionEqualsNewGame) {
  engine from_fen, standard;
  from_fen.new_game(kStart);
  standard.new_game();
  for (int y = 0; y < 8; ++y) {
    for (int x = 0; x < 8; ++x) {
      piece a = from_fen.get_current_board().get_piece(x, y);
      piece b = standard.get_current_board().get_piece(x, y);
      ASSERT_EQ(a.is_valid(), b.is_valid()) << "square " << x << "," << y;
      if (a.is_valid()) {
        EXPECT_EQ(a.get_piece_type(), b.get_piece_type());
        EXPECT_EQ(a.get_piece_color(), b.get_piece_color());
      }
    }
  }
  EXPECT_EQ(legal_uci_moves(standard).size(), 20u);
}

TEST(Fen, PiecePlacement) {
  engine e;
  e.new_game("4k3/8/8/8/8/8/8/R3K2R w KQ - 0 1");
  board& b = e.get_current_board();
  EXPECT_EQ(b.get_num_pieces(), 4);
  piece rook = b.get_piece(0, 0);  // a1
  ASSERT_TRUE(rook.is_valid());
  EXPECT_EQ(rook.get_piece_type(), ROOK);
  EXPECT_EQ(rook.get_piece_color(), piece_color::WHITE);
  piece king = b.get_piece(4, 7);  // e8
  ASSERT_TRUE(king.is_valid());
  EXPECT_EQ(king.get_piece_type(), KING);
  EXPECT_EQ(king.get_piece_color(), piece_color::BLACK);
  EXPECT_FALSE(b.get_piece(4, 4).is_valid());  // e5 is empty
}

TEST(Fen, SideToMoveCastlingAndMoveNumber) {
  engine e;
  e.new_game("r3k2r/8/8/8/8/8/8/R3K2R b Kq - 3 8");
  game_history gs = e.get_current_gamestate();
  EXPECT_EQ(gs.to_move, piece_color::BLACK);
  EXPECT_EQ(gs.castlingrights[(int)piece_color::WHITE], SHORT);
  EXPECT_EQ(gs.castlingrights[(int)piece_color::BLACK], LONG);
  EXPECT_EQ(gs.num_halfmoves, 15);  // move 8, black to move
}

TEST(Fen, EnPassantSquare) {
  engine e;
  e.new_game("4k3/8/8/3pP3/8/8/8/4K3 w - d6 0 2");
  game_history gs = e.get_current_gamestate();
  EXPECT_EQ(std::get<0>(gs.ep_square), 3);  // d
  EXPECT_EQ(std::get<1>(gs.ep_square), 5);  // 6
  EXPECT_TRUE(has_move(e, "e5d6"));         // the en passant capture
}

TEST(Fen, MoveCountersAreOptional) {
  engine e;
  EXPECT_NO_THROW(e.new_game("4k3/8/8/8/8/8/8/4K3 w - -"));
}

TEST(Fen, InvalidInputThrows) {
  engine e;
  EXPECT_THROW(e.new_game(""), std::invalid_argument);
  EXPECT_THROW(e.new_game("nonsense"), std::invalid_argument);
  // only 7 ranks
  EXPECT_THROW(e.new_game("8/8/8/8/8/8/4K3 w - - 0 1"), std::invalid_argument);
  // 9 squares in a rank
  EXPECT_THROW(e.new_game("4k4/8/8/8/8/8/8/4K3 w - - 0 1"),
               std::invalid_argument);
  // unknown piece letter
  EXPECT_THROW(e.new_game("4x3/8/8/8/8/8/8/4K3 w - - 0 1"),
               std::invalid_argument);
  EXPECT_THROW(e.new_game("4k3/8/8/8/8/8/8/4K3 x - - 0 1"),
               std::invalid_argument);
  EXPECT_THROW(e.new_game("4k3/8/8/8/8/8/8/4K3 w KX - 0 1"),
               std::invalid_argument);
  EXPECT_THROW(e.new_game("4k3/8/8/8/8/8/8/4K3 w - z9 0 1"),
               std::invalid_argument);
}

TEST(Fen, LoadedCheckmateIsGameOver) {
  // Fool's mate: 1. f3 e5 2. g4 Qh4#
  engine e;
  e.new_game("rnb1kbnr/pppp1ppp/8/4p3/6Pq/5P2/PPPPP2P/RNBQKBNR w KQkq - 1 3");
  game_history gs = e.get_current_gamestate();
  EXPECT_TRUE(gs.is_check);
  EXPECT_EQ(gs.result, BLACK_WIN);
  EXPECT_TRUE(legal_uci_moves(e).empty());
}

TEST(Fen, LoadedStalemateIsDraw) {
  engine e;
  e.new_game("7k/5Q2/6K1/8/8/8/8/8 b - - 0 1");
  game_history gs = e.get_current_gamestate();
  EXPECT_FALSE(gs.is_check);
  EXPECT_EQ(gs.result, DRAW);
}

}  // namespace
