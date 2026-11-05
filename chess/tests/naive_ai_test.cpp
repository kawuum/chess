// Tests for naive_ai. They hold for the stub and must keep holding for the
// real implementation.
#include "../ai/naive/naive_ai.hpp"

#include <algorithm>

#include "../ai/ai_registry.hpp"
#include "test_helpers.hpp"

namespace {

const char* const kStart =
    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

TEST(NaiveAi, PlaysOnlyLegalMoves) {
  engine e;
  e.new_game();
  naive_ai ai(&e);
  for (int ply = 0; ply < 100; ++ply) {
    if (e.get_current_gamestate().result != RUNNING) break;
    std::string uci = to_uci(ai.next_move());
    ASSERT_TRUE(has_move(e, uci)) << "illegal move " << uci;
    play(e, uci);
  }
}

TEST(NaiveAi, LeavesThePositionUnchanged) {
  // next_move() may try moves, but must take all of them back.
  engine e;
  e.new_game(
      "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");
  board before = e.get_current_board();
  int halfmoves = e.get_current_gamestate().num_halfmoves;
  naive_ai ai(&e);
  ai.next_move();
  EXPECT_TRUE(e.get_current_board() == before);
  EXPECT_EQ(e.get_current_gamestate().num_halfmoves, halfmoves);
  EXPECT_EQ(e.get_color_to_move(), piece_color::WHITE);
}

TEST(NaiveAi, NoMoveWhenGameIsOver) {
  engine e;
  e.new_game("rnb1kbnr/pppp1ppp/8/4p3/6Pq/5P2/PPPPP2P/RNBQKBNR w KQkq - 1 3");
  naive_ai ai(&e);
  EXPECT_EQ(ai.next_move().type, NO_MOVE);
}

TEST(NaiveAi, StartPositionIsEven) {
  // The start position is symmetric: neither side is better.
  engine e;
  e.new_game(kStart);
  naive_ai ai(&e);
  EXPECT_EQ(ai.evaluate(e.get_current_board(), piece_color::WHITE), 0);
  EXPECT_EQ(ai.evaluate(e.get_current_board(), piece_color::BLACK), 0);
}

TEST(NaiveAi, EvaluationIsSymmetric) {
  // Every point for one side is a point against the other: what is good for
  // White by n is bad for Black by n.
  engine e;
  e.new_game(
      "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");
  naive_ai ai(&e);
  const board& b = e.get_current_board();
  EXPECT_EQ(ai.evaluate(b, piece_color::WHITE),
            -ai.evaluate(b, piece_color::BLACK));
}

TEST(NaiveAi, IsInTheRegistry) {
  const auto& ais = available_ais();
  EXPECT_TRUE(std::any_of(ais.begin(), ais.end(), [](const ai_info& ai) {
    return ai.name == "Naive AI";
  }));
}

}  // namespace
