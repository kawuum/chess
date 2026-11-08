// engine::perform_move() only accepts legal moves. Anything else throws
// std::invalid_argument and leaves the position unchanged.
#include <algorithm>
#include <random>
#include <stdexcept>
#include <vector>

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

// A promotion move of the pawn on a7, built by hand: the target square a8 is
// right, the promotion piece is whatever the test sets.
move promotion_move(piece_type promotion) {
  move m = make_move(kWhitePawn, 0, 6, 0, 7, PROMOTION);
  m.promotion = promotion;
  return m;
}

TEST(MoveValidation, FourMovesPerPromotion) {
  // Every promotion piece is its own legal move.
  engine e;
  e.new_game(kPromotionFen);
  std::vector<std::string> moves;
  for (const move& m : e.get_legal_moves(0, 6)) moves.push_back(to_uci(m));
  std::sort(moves.begin(), moves.end());
  EXPECT_EQ(moves,
            (std::vector<std::string>{"a7a8b", "a7a8n", "a7a8q", "a7a8r"}));
}

TEST(MoveValidation, PromotionWithoutPieceThrows) {
  // move::promotion defaults to KING, which means "none".
  engine e;
  e.new_game(kPromotionFen);
  move m = make_move(kWhitePawn, 0, 6, 0, 7, PROMOTION);
  EXPECT_THROW(e.perform_move(m), std::invalid_argument);
  EXPECT_EQ(e.get_current_board().get_piece(0, 6).get_piece_type(), PAWN);
}

TEST(MoveValidation, PromotionToKingOrPawnThrows) {
  engine e;
  e.new_game(kPromotionFen);
  EXPECT_THROW(e.perform_move(promotion_move(KING)), std::invalid_argument);
  EXPECT_THROW(e.perform_move(promotion_move(PAWN)), std::invalid_argument);
  EXPECT_EQ(e.get_current_board().get_piece(0, 6).get_piece_type(), PAWN);
}

TEST(MoveValidation, PromotionFieldIsIgnoredForNormalMoves) {
  // The promotion piece only means something for promotions.
  engine e;
  e.new_game();
  move m = make_move(kWhitePawn, 4, 1, 4, 3, MOVE);
  m.promotion = QUEEN;
  e.perform_move(m);
  EXPECT_EQ(e.get_current_board().get_piece(4, 3).get_piece_type(), PAWN);
}

TEST(MoveValidation, UnderpromotionToKnight) {
  engine e;
  e.new_game(kPromotionFen);
  e.perform_move(promotion_move(KNIGHT));
  piece p = e.get_current_board().get_piece(0, 7);  // a8
  ASSERT_TRUE(p.is_valid());
  EXPECT_EQ(p.get_piece_type(), KNIGHT);
  EXPECT_EQ(p.get_piece_color(), piece_color::WHITE);
}

// The cheap getters for the search (no copy of the game state, moves from
// the cache) must give the same as the copying ones, also after undo_move().
TEST(MoveValidation, CheapGettersMatchTheCopyingOnes) {
  engine e;
  e.new_game();
  std::mt19937 rng(5);
  for (int ply = 0; ply < 120; ++ply) {
    game_history copy = e.get_current_gamestate();
    const game_history& state = e.current_state();
    ASSERT_TRUE(state.curr_board == copy.curr_board);
    ASSERT_EQ(state.num_halfmoves, copy.num_halfmoves);
    ASSERT_EQ(state.result, copy.result);
    const std::vector<move> cached = e.get_all_legal_moves();
    ASSERT_EQ(cached, e.get_all_legal_moves(&copy)) << "ply " << ply;
    if (cached.empty()) break;
    // The reference stays valid while later moves are played and undone.
    e.perform_move(cached[rng() % cached.size()]);
    const std::vector<move> replies = e.get_all_legal_moves();
    if (!replies.empty()) {
      e.perform_move(replies[0]);
      e.undo_move();
    }
    e.undo_move();
    ASSERT_EQ(state.num_halfmoves, copy.num_halfmoves);
    ASSERT_EQ(e.get_all_legal_moves(), cached);
    e.perform_move(cached[rng() % cached.size()]);
  }
}

}  // namespace
