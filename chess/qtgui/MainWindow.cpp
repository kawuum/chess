#include "MainWindow.h"
#include <QMenu>
#include <QMenuBar>

MainWindow::MainWindow(int* parent) : QMainWindow(parent)
{
  QMenu *game = menuBar()->addMenu("&Game");
  QAction *new_game = new QAction("&New Game", this);
  game->addAction(new_game);
  QAction *undo_move = new QAction("&Undo Move", this);
  game->addAction(undo_move);
  QAction *add_ai = new QAction("&Add AI", this);
  game->addAction(add_ai);
  QAction *remove_ai = new QAction("&Remove AI", this);
  game->addAction(remove_ai);
  QAction *quit = new QAction("&Quit", this);
  game->addAction(quit);
  connect(quit, &QAction::triggered, qApp, QApplication::quit);
}
