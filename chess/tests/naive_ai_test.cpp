// Tests for naive_ai: the rules every AI must follow, properties of the
// rating, and a few positions where the right move is clear even when
// looking only one half move ahead.
#include "../ai/naive/naive_ai.hpp"

#include <algorithm>

#include "../ai/ai_registry.hpp"
#include "../ai/random/random_ai.hpp"
#include "../engine/evaluation.hpp"
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

TEST(NaiveAi, TakesMateInOne) {
  // White: Kg6, Qf1; Black: Kh8. Qc8, Qd8, Qe8 and Qf8 are mate; Qf7 would
  // be stalemate. Every seed must find a mate.
  for (uint32_t seed = 1; seed <= 5; ++seed) {
    engine e;
    e.new_game("7k/8/6K1/8/8/8/8/5Q2 w - - 0 1");
    naive_ai ai(&e, "naive", seed);
    e.perform_move(ai.next_move());
    EXPECT_EQ(e.get_current_gamestate().result, WHITE_WIN) << "seed " << seed;
  }
}

TEST(NaiveAi, CapturesAnUndefendedQueen) {
  engine e;
  e.new_game("4k3/8/8/3q4/4P3/8/8/4K3 w - - 0 1");
  naive_ai ai(&e, "naive", 1);
  EXPECT_EQ(to_uci(ai.next_move()), "e4d5");
}

TEST(NaiveAi, DoesNotLeaveItsQueenWhereItCanBeTaken) {
  // The white queen on d4 is attacked by the pawn c5. Taking that pawn
  // (Qxc5) would win a pawn, but then the pawn d6 takes the queen. After the
  // AI's move no white piece may be under attack.
  for (uint32_t seed = 1; seed <= 5; ++seed) {
    engine e;
    e.new_game("4k3/8/3p4/2p5/3Q4/8/8/4K3 w - - 0 1");
    naive_ai ai(&e, "naive", seed);
    e.perform_move(ai.next_move());
    EXPECT_EQ(
        evaluation::threatened_value(e.get_current_board(), piece_color::BLACK),
        0)
        << "seed " << seed;
  }
}

TEST(NaiveAi, SameSeedPlaysSameGame) {
  engine e1, e2;
  e1.new_game();
  e2.new_game();
  naive_ai a(&e1, "a", 7), b(&e2, "b", 7);
  for (int ply = 0; ply < 40; ++ply) {
    if (e1.get_current_gamestate().result != RUNNING) break;
    move m1 = a.next_move();
    move m2 = b.next_move();
    ASSERT_EQ(to_uci(m1), to_uci(m2)) << "ply " << ply;
    e1.perform_move(m1);
    e2.perform_move(m2);
  }
}

// Plays one game of naive_ai against random_ai and returns the result
// (RUNNING if the game is not decided after max_plies half moves).
game_result naive_against_random(piece_color naive_color, uint32_t seed,
                                 int max_plies) {
  engine e;
  e.new_game();
  naive_ai naive(&e, "naive", seed);
  random_ai random(&e, "random", seed);
  for (int ply = 0; ply < max_plies; ++ply) {
    if (e.get_current_gamestate().result != RUNNING) break;
    bool naive_to_move = e.get_color_to_move() == naive_color;
    e.perform_move(naive_to_move ? naive.next_move() : random.next_move());
  }
  return e.get_current_gamestate().result;
}

TEST(NaiveAi, BeatsRandomAi) {
  // Regression test for the strength: of 100 games (fixed seeds, colors
  // alternating) the naive AI must win at least 90. With these seeds it won
  // 97 when the test was written. The games are the same on every run, but
  // any change to the AI, random_ai or the order of the generated moves
  // gives other games, so the number may move a little; 90 leaves room.
  // Counter-check: with the material counted the wrong way round it wins
  // only 52.
  constexpr int kGames = 100;
  constexpr int kMinWins = 90;
  constexpr int kMaxPlies = 300;
  int wins = 0;
  for (int game = 0; game < kGames; ++game) {
    piece_color naive_color =
        game % 2 == 0 ? piece_color::WHITE : piece_color::BLACK;
    game_result win = game % 2 == 0 ? WHITE_WIN : BLACK_WIN;
    if (naive_against_random(naive_color, 1000 + game, kMaxPlies) == win) {
      ++wins;
    }
  }
  EXPECT_GE(wins, kMinWins);
}

TEST(NaiveAi, IsInTheRegistry) {
  const auto& ais = available_ais();
  EXPECT_TRUE(std::any_of(ais.begin(), ais.end(), [](const ai_info& ai) {
    return ai.name == "Naive AI";
  }));
}

}  // namespace
