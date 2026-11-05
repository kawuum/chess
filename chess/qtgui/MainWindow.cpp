#include "MainWindow.hpp"

#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QMenuBar>
#include <QStatusBar>

#include "ChessBoardWidget.hpp"
#include "SimulationDialog.hpp"

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent), board_(new ChessBoardWidget(this)) {
  setWindowTitle("Chess");
  setCentralWidget(board_);

  // Every QAction gets the window as parent, so Qt deletes it with the window.
  QMenu* game = menuBar()->addMenu("&Game");

  QAction* newGame = game->addAction("&New Game");
  newGame->setShortcut(QKeySequence::New);
  connect(newGame, &QAction::triggered, board_, &ChessBoardWidget::newGame);

  QAction* undo = game->addAction("&Undo Move");
  undo->setShortcut(QKeySequence::Undo);
  connect(undo, &QAction::triggered, board_, &ChessBoardWidget::undoMove);

  game->addSeparator();

  QAction* simulate = game->addAction("&Simulate Games...");
  connect(simulate, &QAction::triggered, this, &MainWindow::showSimulation);

  game->addSeparator();

  QAction* quit = game->addAction("&Quit");
  quit->setShortcut(QKeySequence::Quit);
  connect(quit, &QAction::triggered, qApp, &QApplication::quit);

  QMenu* players = menuBar()->addMenu("&Players");
  addPlayerMenu(players, "&White", piece_color::WHITE);
  addPlayerMenu(players, "&Black", piece_color::BLACK);

  QMenu* options = menuBar()->addMenu("&Options");
  addAiSpeedMenu(options);

  connect(board_, &ChessBoardWidget::statusChanged, statusBar(),
          [this](const QString& text) { statusBar()->showMessage(text); });
  // The board emitted its first status in its constructor, before this
  // connection existed, so show it once by hand.
  statusBar()->showMessage(board_->statusText());
}

void MainWindow::addPlayerMenu(QMenu* parent, const QString& title,
                               piece_color color) {
  QMenu* menu = parent->addMenu(title);
  // An exclusive QActionGroup works like radio buttons: checking one entry
  // unchecks the others.
  auto* group = new QActionGroup(menu);

  QAction* human = menu->addAction("Human");
  human->setCheckable(true);
  human->setChecked(true);
  group->addAction(human);
  connect(human, &QAction::triggered, board_,
          [this, color] { board_->setPlayer(color, nullptr); });

  menu->addSeparator();
  for (const ai_info& ai : available_ais()) {
    QAction* action = menu->addAction(QString::fromStdString(ai.name));
    action->setCheckable(true);
    group->addAction(action);
    // The pointer stays valid: available_ais() is a static list.
    const ai_info* info = &ai;
    connect(action, &QAction::triggered, board_,
            [this, color, info] { board_->setPlayer(color, info); });
  }
}

void MainWindow::showSimulation() {
  // Created on first use and then kept: closing only hides it, so the last
  // results are still there when it is opened again. Not modal: the board
  // stays usable while games are simulated.
  if (simulation_ == nullptr) simulation_ = new SimulationDialog(this);
  simulation_->show();
  simulation_->raise();
  simulation_->activateWindow();
}

void MainWindow::addAiSpeedMenu(QMenu* parent) {
  QMenu* menu = parent->addMenu("&AI vs AI speed");
  auto* group = new QActionGroup(menu);  // radio buttons, see addPlayerMenu

  struct Choice {
    const char* text;
    int delayMs;
  };
  const Choice choices[] = {
      {"Wait for click", ChessBoardWidget::kWaitForClick},
      {"0.3 s per move", 300},
      {"1 s per move", 1000},
      {"2 s per move", 2000},
  };
  for (const Choice& choice : choices) {
    QAction* action = menu->addAction(choice.text);
    action->setCheckable(true);
    group->addAction(action);
    int delay = choice.delayMs;
    connect(action, &QAction::triggered, board_,
            [this, delay] { board_->setAiVsAiDelay(delay); });
  }
  // The widget starts with "wait for click".
  group->actions().first()->setChecked(true);
}
