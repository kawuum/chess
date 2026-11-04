// engine::perform_move() only accepts legal moves. Anything else throws
// std::invalid_argument and leaves the position unchanged.
#include <stdexcept>

#include "test_helpers.hpp"

namespace {

const piece kWhitePawn(PAWN, piece_color::WHITE);

// A move built by hand, as a buggy caller might do it.
move make_move(piece mover, int fx, int fy, int tx, int ty, move_type type) {
  return move{mover,
              static_cast<uint8_t>(fx),
              static_cast<uint8_t>(fy),
              static_cast<uint8_t>(tx),
              static_cast<uint8_t>(ty),
              type};
}

void expect_start_position(engine& e) {
  EXPECT_EQ(e.get_current_gamestate().num_halfmoves, 0);
  EXPECT_EQ(e.get_color_to_move(), piece_color::WHITE);
  EXPECT_EQ(legal_uci_moves(e).size(), 20u);
  EXPECT_EQ(e.get_current_board().get_piece(4, 1).get_piece_type(), PAWN);
}

TEST(MoveValidation, IllegalTargetThrowsAndKeepsPosition) {
  engine e;
  e.new_game();
  move m = make_move(kWhitePawn, 4, 1, 4, 4, MOVE);  // e2-e5
  EXPECT_THROW(e.perform_move(m), std::invalid_argument);
  expect_start_position(e);
}

TEST(MoveValidation, OpponentPieceThrows) {
  engine e;
  e.new_game();
  move m = make_move(piece(PAWN, piece_color::BLACK), 4, 6, 4, 4, MOVE);
  EXPECT_THROW(e.perform_move(m), std::invalid_argument);
  expect_start_position(e);
}

TEST(MoveValidation, EmptySquareThrows) {
  engine e;
  e.new_game();
  move m = make_move(kWhitePawn, 4, 3, 4, 4, MOVE);  // e4-e5, e4 is empty
  EXPECT_THROW(e.perform_move(m), std::invalid_argument);
  expect_start_position(e);
}

TEST(MoveValidation, NoMovesAfterGameOver) {
  engine e;
  e.new_game("rnb1kbnr/pppp1ppp/8/4p3/6Pq/5P2/PPPPP2P/RNBQKBNR w KQkq - 1 3");
  move m = make_move(kWhitePawn, 0, 1, 0, 2, MOVE);  // a2-a3 after mate
  EXPECT_THROW(e.perform_move(m), std::invalid_argument);
}

TEST(MoveValidation, UsesTheEnginesMoveData) {
  // The caller claims e2-e4 is a capture. The engine uses its own move list,
  // so the wrong type does no harm.
  engine e;
  e.new_game();
  move m = make_move(kWhitePawn, 4, 1, 4, 3, CAPTURE);
  e.perform_move(m);
  board& b = e.get_current_board();
  EXPECT_EQ(b.get_piece(4, 3).get_piece_type(), PAWN);
  EXPECT_FALSE(b.get_piece(4, 1).is_valid());
  EXPECT_EQ(b.get_num_pieces(), 32);
}

// --- Promotion --------------------------------------------------------------

const char* const kPromotionFen = "4k3/P7/8/8/8/8/8/4K3 w - - 0 1";

move promotion_move(engine& e) {
  std::vector<move> moves = e.get_legal_moves(0, 6);  // pawn on a7
  EXPECT_EQ(moves.size(), 1u);
  return moves.at(0);
}

TEST(MoveValidation, PromotionWithoutPieceThrows) {
  engine e;
  e.new_game(kPromotionFen);
  move m = promotion_move(e);
  EXPECT_THROW(e.perform_move(m), std::invalid_argument);
  EXPECT_EQ(e.get_current_board().get_piece(0, 6).get_piece_type(), PAWN);
}

TEST(MoveValidation, PromotionToKingOrPawnThrows) {
  engine e;
  e.new_game(kPromotionFen);
  move m = promotion_move(e);
  piece king(KING, piece_color::WHITE), pawn(PAWN, piece_color::WHITE);
  EXPECT_THROW(e.perform_move(m, king), std::invalid_argument);
  EXPECT_THROW(e.perform_move(m, pawn), std::invalid_argument);
}

TEST(MoveValidation, PromotionToOpponentColorThrows) {
  engine e;
  e.new_game(kPromotionFen);
  move m = promotion_move(e);
  piece black_queen(QUEEN, piece_color::BLACK);
  EXPECT_THROW(e.perform_move(m, black_queen), std::invalid_argument);
}

TEST(MoveValidation, PromotionPieceForNormalMoveThrows) {
  engine e;
  e.new_game();
  move m = make_move(kWhitePawn, 4, 1, 4, 3, MOVE);
  piece queen(QUEEN, piece_color::WHITE);
  EXPECT_THROW(e.perform_move(m, queen), std::invalid_argument);
  expect_start_position(e);
}

TEST(MoveValidation, UnderpromotionToKnight) {
  engine e;
  e.new_game(kPromotionFen);
  move m = promotion_move(e);
  piece knight(KNIGHT, piece_color::WHITE);
  e.perform_move(m, knight);
  piece p = e.get_current_board().get_piece(0, 7);  // a8
  ASSERT_TRUE(p.is_valid());
  EXPECT_EQ(p.get_piece_type(), KNIGHT);
  EXPECT_EQ(p.get_piece_color(), piece_color::WHITE);
}

}  // namespace
