// GUI tests: drive MainWindow and ChessBoardWidget with simulated mouse
// clicks. They run without a screen on Qt's "offscreen" platform.
#include <gtest/gtest.h>

#include <QAction>
#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QMenu>
#include <QMenuBar>
#include <QPushButton>
#include <QSpinBox>
#include <QStatusBar>
#include <QTest>
#include <algorithm>
#include <ostream>

#include "../ai/ai_registry.hpp"
#include "../qtgui/ChessBoardWidget.hpp"
#include "../qtgui/MainWindow.hpp"
#include "../qtgui/SimulationDialog.hpp"

// Lets GoogleTest print a QString as text when a test fails, instead of its
// raw bytes. GoogleTest finds this function by name (it must be in the same
// namespace as QString, the global one).
void PrintTo(const QString& s, std::ostream* os) {
  *os << '"' << s.toStdString() << '"';
}

namespace {

// Center of a board square in widget coordinates. Same geometry as
// ChessBoardWidget: square board, centered, white at the bottom.
QPoint center(QWidget& w, int x, int y) {
  int s = std::min(w.width(), w.height()) / 8;
  int ox = (w.width() - 8 * s) / 2;
  int oy = (w.height() - 8 * s) / 2;
  return QPoint(ox + x * s + s / 2, oy + (7 - y) * s + s / 2);
}

QAction* find_action(QMenu* menu, const QString& text) {
  for (QAction* a : menu->actions()) {
    if (a->text() == text) return a;
  }
  return nullptr;
}

class Gui : public testing::Test {
 protected:
  void SetUp() override {
    window.resize(640, 700);
    window.show();
    board = window.findChild<ChessBoardWidget*>();
    ASSERT_NE(board, nullptr);
  }

  void click(int x, int y) {
    QTest::mouseClick(board, Qt::LeftButton, Qt::NoModifier,
                      center(*board, x, y));
  }

  QString status() { return window.statusBar()->currentMessage(); }

  // The submenu Players > White/Black.
  QMenu* player_menu(const QString& color) {
    for (QAction* a : window.menuBar()->actions()) {
      if (a->text() == "&Players") {
        QAction* sub = find_action(a->menu(), color);
        return sub ? sub->menu() : nullptr;
      }
    }
    return nullptr;
  }

  // An entry of Options > AI vs AI speed.
  QAction* ai_speed_action(const QString& text) {
    for (QAction* a : window.menuBar()->actions()) {
      if (a->text() == "&Options") {
        QAction* sub = find_action(a->menu(), "&AI vs AI speed");
        return sub ? find_action(sub->menu(), text) : nullptr;
      }
    }
    return nullptr;
  }

  // Opens Game > Simulate Games and sets up the dialog like a user would.
  SimulationDialog* open_simulation(const QString& first, const QString& second,
                                    int games) {
    for (QAction* a : window.menuBar()->actions()) {
      if (a->text() == "&Game") {
        find_action(a->menu(), "&Simulate Games...")->trigger();
      }
    }
    auto* dialog = window.findChild<SimulationDialog*>();
    if (dialog == nullptr) return nullptr;
    dialog->findChild<QComboBox*>("firstAi")->setCurrentText(first);
    dialog->findChild<QComboBox*>("secondAi")->setCurrentText(second);
    dialog->findChild<QSpinBox*>("games")->setValue(games);
    return dialog;
  }

  void click_button(QWidget* parent, const char* name) {
    QTest::mouseClick(parent->findChild<QPushButton*>(name), Qt::LeftButton);
  }

  // Both colors played by the Random AI, chosen in the menu like a user does.
  void random_ai_for_both() {
    find_action(player_menu("&White"), "Random AI")->trigger();
    find_action(player_menu("&Black"), "Random AI")->trigger();
  }

  MainWindow window;
  ChessBoardWidget* board = nullptr;
};

TEST_F(Gui, OpensAsASingleWindow) {
  // The old board opened every square as its own window.
  int visible = 0;
  for (QWidget* w : QApplication::topLevelWidgets()) {
    if (w->isVisible()) ++visible;
  }
  EXPECT_EQ(visible, 1);
}

TEST_F(Gui, StartsWithWhiteToMove) { EXPECT_EQ(status(), "White to move"); }

TEST_F(Gui, PieceImagesAreLoaded) {
  // The pixel in the middle of e2 belongs to the pawn image, not to the
  // square. If the embedded images were missing, it would show the square.
  QImage img = board->grab().toImage();
  QColor pawn = img.pixelColor(center(*board, 4, 1));
  QColor empty_light = img.pixelColor(center(*board, 4, 3));  // e4
  EXPECT_NE(pawn, empty_light);
}

TEST_F(Gui, ClickTwiceMovesAPiece) {
  click(4, 1);  // e2
  click(4, 3);  // e4
  EXPECT_EQ(status(), "Black to move");
}

TEST_F(Gui, IllegalTargetDoesNotMove) {
  click(4, 1);  // e2
  click(4, 4);  // e5: too far for a pawn
  EXPECT_EQ(status(), "White to move");
}

TEST_F(Gui, UndoTakesTheMoveBack) {
  click(4, 1);
  click(4, 3);
  board->undoMove();
  EXPECT_EQ(status(), "White to move");
}

TEST_F(Gui, PlayersMenuListsHumanAndAllAis) {
  QMenu* black = player_menu("&Black");
  ASSERT_NE(black, nullptr);
  EXPECT_NE(find_action(black, "Human"), nullptr);
  for (const ai_info& ai : available_ais()) {
    EXPECT_NE(find_action(black, QString::fromStdString(ai.name)), nullptr);
  }
  EXPECT_TRUE(find_action(black, "Human")->isChecked());
}

TEST_F(Gui, AiChosenInMenuAnswersAMove) {
  QMenu* black = player_menu("&Black");
  ASSERT_NE(black, nullptr);
  find_action(black, "Random AI")->trigger();
  click(4, 1);
  click(4, 3);
  EXPECT_EQ(status(), "Black to move (Random AI)");
  // The AI moves after a short delay (QTimer), so wait for it.
  EXPECT_TRUE(
      QTest::qWaitFor([&] { return status() == "White to move"; }, 2000));
}

TEST_F(Gui, AiSpeedStartsWithWaitForClick) {
  QAction* wait = ai_speed_action("Wait for click");
  ASSERT_NE(wait, nullptr);
  EXPECT_TRUE(wait->isChecked());
}

TEST_F(Gui, AiVsAiWaitsForAClick) {
  random_ai_for_both();
  // Switching White to an AI first started the timer of a game against a
  // human. It runs out during this wait, but must not make a move.
  QTest::qWait(600);
  EXPECT_EQ(status(),
            "White to move (Random AI) - click the board for the next move");
  click(4, 4);  // any square
  EXPECT_EQ(status(),
            "Black to move (Random AI) - click the board for the next move");
  QTest::qWait(600);  // still nothing happens without a click
  EXPECT_EQ(status(),
            "Black to move (Random AI) - click the board for the next move");
}

TEST_F(Gui, AiVsAiWithDelayPlaysByItself) {
  random_ai_for_both();
  // Chosen while the game waits for a click: it goes on by itself.
  ai_speed_action("0.3 s per move")->trigger();
  EXPECT_TRUE(QTest::qWaitFor(
      [&] { return status().startsWith("Black to move"); }, 2000));
  EXPECT_TRUE(QTest::qWaitFor(
      [&] { return status().startsWith("White to move"); }, 2000));
  EXPECT_FALSE(status().contains("click"));
}

TEST_F(Gui, NaiveAiTakesAHangingQueen) {
  // After 1.e4 d5 the human plays 2.Qg4??; the naive AI (Black) must answer
  // with Bc8xg4.
  find_action(player_menu("&Black"), "Naive AI")->trigger();
  board->loadPosition(
      "rnbqkbnr/ppp1pppp/8/3p4/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 2");
  click(3, 0);  // d1
  click(6, 3);  // g4
  ASSERT_TRUE(QTest::qWaitFor(
      [&] { return status().startsWith("White to move"); }, 2000));
  piece p = board->pieceAt(6, 3);  // g4
  ASSERT_TRUE(p.is_valid());
  EXPECT_EQ(p.get_piece_type(), BISHOP);
  EXPECT_EQ(p.get_piece_color(), piece_color::BLACK);
}

TEST_F(Gui, SimulationPlaysGamesInTheBackground) {
  SimulationDialog* dialog = open_simulation("Naive AI", "Random AI", 10);
  ASSERT_NE(dialog, nullptr);
  EXPECT_TRUE(dialog->isVisible());
  click_button(dialog, "start");
  // The end is reported through the event loop, so right after the click
  // the simulation is still running, however fast the games are.
  EXPECT_TRUE(dialog->isRunning());
  ASSERT_TRUE(QTest::qWaitFor([&] { return !dialog->isRunning(); }, 30000));
  EXPECT_TRUE(
      dialog->resultText().contains("10 games: Naive AI against Random AI"))
      << dialog->resultText().toStdString();
  EXPECT_TRUE(dialog->resultText().contains("Naive AI wins:"));
  EXPECT_EQ(status(), "White to move");  // the board is not touched
}

TEST_F(Gui, SimulationCanBeStopped) {
  SimulationDialog* dialog = open_simulation("Random AI", "Random AI", 100000);
  ASSERT_NE(dialog, nullptr);
  click_button(dialog, "start");
  // Wait until some games are played, then stop.
  ASSERT_TRUE(QTest::qWaitFor(
      [&] { return dialog->resultText().contains("games:"); }, 10000));
  click_button(dialog, "stop");
  ASSERT_TRUE(QTest::qWaitFor([&] { return !dialog->isRunning(); }, 10000));
  EXPECT_TRUE(dialog->resultText().startsWith("Stopped after"))
      << dialog->resultText().toStdString();
}

TEST_F(Gui, DialogCanBeDeletedWhileSimulating) {
  // Deleting the dialog while 100000 games run (e.g. when the program is
  // closed) must stop its thread and wait for it. A thread that went on
  // would use the deleted dialog: Qt aborts if a running QThread is
  // destroyed, and a thread left running would write to freed memory during
  // the wait below (the ASan build reports that).
  SimulationDialog* dialog = open_simulation("Random AI", "Random AI", 100000);
  ASSERT_NE(dialog, nullptr);
  click_button(dialog, "start");
  QTest::qWait(50);
  ASSERT_TRUE(dialog->isRunning());
  delete dialog;
  QTest::qWait(200);
  EXPECT_EQ(window.findChild<SimulationDialog*>(), nullptr);
}

TEST_F(Gui, RandomSeedCheckboxGreysOutTheSeed) {
  SimulationDialog* dialog = open_simulation("Random AI", "Naive AI", 10);
  ASSERT_NE(dialog, nullptr);
  for (const char* name : {"first", "second"}) {
    auto* random = dialog->findChild<QCheckBox*>(QString(name) + "RandomSeed");
    auto* seed = dialog->findChild<QSpinBox*>(QString(name) + "Seed");
    EXPECT_TRUE(random->isChecked()) << name;  // the default
    EXPECT_FALSE(seed->isEnabled()) << name;
    random->setChecked(false);
    EXPECT_TRUE(seed->isEnabled()) << name;
  }
}

TEST_F(Gui, SeedRowOnlyForAisThatUseRandom) {
  SimulationDialog* dialog = open_simulation("Random AI", "Random AI", 10);
  ASSERT_NE(dialog, nullptr);
  auto* combo = dialog->findChild<QComboBox*>("firstAi");
  auto* row = dialog->findChild<QWidget*>("firstSeedRow");
  for (const ai_info& ai : available_ais()) {
    combo->setCurrentText(QString::fromStdString(ai.name));
    EXPECT_EQ(row->isVisible(), ai.uses_random) << ai.name;
  }
}

TEST_F(Gui, RandomSeedIsShownAndRepeatsTheSimulation) {
  SimulationDialog* dialog = open_simulation("Random AI", "Naive AI", 6);
  ASSERT_NE(dialog, nullptr);
  auto run = [&] {
    click_button(dialog, "start");
    EXPECT_TRUE(QTest::qWaitFor([&] { return !dialog->isRunning(); }, 30000));
    return dialog->resultText();
  };
  QString first_run = run();  // with random seeds, drawn now
  // The drawn seeds are in the fields and named in the result.
  int seed1 = dialog->findChild<QSpinBox*>("firstSeed")->value();
  int seed2 = dialog->findChild<QSpinBox*>("secondSeed")->value();
  EXPECT_TRUE(first_run.contains(
      QString("Seeds: first AI %1, second AI %2").arg(seed1).arg(seed2)))
      << first_run.toStdString();
  // With these seeds fixed, the same games are played again.
  dialog->findChild<QCheckBox*>("firstRandomSeed")->setChecked(false);
  dialog->findChild<QCheckBox*>("secondRandomSeed")->setChecked(false);
  EXPECT_EQ(run(), first_run);
}

// White pawn on a7, one step before promotion.
// The rook on h1 keeps enough material on the board: after a promotion to a
// knight, king and knight against king would be a draw, and the "game over"
// dialog would wait for a click forever.
const char* const kPromotionFen = "4k3/P7/8/8/8/8/8/4K2R w - - 0 1";

TEST_F(Gui, PromotionPlaysTheChosenPiece) {
  // Instead of the dialog, a function chooses the knight.
  int asked = 0;
  board->setPromotionChooser([&asked]() -> std::optional<piece_type> {
    ++asked;
    return KNIGHT;
  });
  board->loadPosition(kPromotionFen);
  click(0, 6);  // a7
  click(0, 7);  // a8
  EXPECT_EQ(asked, 1);
  piece p = board->pieceAt(0, 7);
  ASSERT_TRUE(p.is_valid());
  EXPECT_EQ(p.get_piece_type(), KNIGHT);
  EXPECT_EQ(p.get_piece_color(), piece_color::WHITE);
  EXPECT_EQ(status(), "Black to move");
}

TEST_F(Gui, CancelledPromotionKeepsThePosition) {
  board->setPromotionChooser([] { return std::optional<piece_type>(); });
  board->loadPosition(kPromotionFen);
  click(0, 6);
  click(0, 7);
  EXPECT_EQ(board->pieceAt(0, 6).get_piece_type(), PAWN);  // still on a7
  EXPECT_FALSE(board->pieceAt(0, 7).is_valid());           // a8 empty
  EXPECT_EQ(status(), "White to move");
}

}  // namespace

int main(int argc, char** argv) {
  // No screen needed. A real display can still be chosen by setting
  // QT_QPA_PLATFORM before starting the tests.
  if (qEnvironmentVariableIsEmpty("QT_QPA_PLATFORM")) {
    qputenv("QT_QPA_PLATFORM", "offscreen");
  }
  testing::InitGoogleTest(&argc, argv);
  QApplication app(argc, argv);
  return RUN_ALL_TESTS();
}
