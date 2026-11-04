#include "MainWindow.hpp"

#include <QAction>
#include <QApplication>
#include <QMenuBar>
#include <QStatusBar>

#include "ChessBoardWidget.hpp"

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

  QAction* whiteAi = game->addAction("&White played by AI");
  whiteAi->setCheckable(true);
  connect(whiteAi, &QAction::toggled, board_,
          [this](bool on) { board_->setAiPlays(piece_color::WHITE, on); });

  QAction* blackAi = game->addAction("&Black played by AI");
  blackAi->setCheckable(true);
  connect(blackAi, &QAction::toggled, board_,
          [this](bool on) { board_->setAiPlays(piece_color::BLACK, on); });

  game->addSeparator();

  QAction* quit = game->addAction("&Quit");
  quit->setShortcut(QKeySequence::Quit);
  connect(quit, &QAction::triggered, qApp, &QApplication::quit);

  connect(board_, &ChessBoardWidget::statusChanged, statusBar(),
          [this](const QString& text) { statusBar()->showMessage(text); });
  // The board emitted its first status in its constructor, before this
  // connection existed, so show it once by hand.
  statusBar()->showMessage(board_->statusText());
}
