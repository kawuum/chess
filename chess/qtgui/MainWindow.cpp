#include "MainWindow.h"
#include <QMenu>
#include <QMenuBar>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent)
{
  QMenu *game = menuBar()->addMenu("&Game");
  QAction *new_game = new QAction("&New Game", this);
  //new_game->setIcon(QIcon(":/images/new.png"));
  //new_game->setShortcut(QKeySequence::New);
  //new_game->setStatusTip(tr("Start a new game"));
  game->addAction(new_game);
  QAction *undo_move = new QAction("&Undo Move", this);
  game->addAction(undo_move);
  QAction *add_ai = new QAction("&Add AI", this);
  game->addAction(add_ai);
  QAction *remove_ai = new QAction("&Remove AI", this);
  game->addAction(remove_ai);
  game->addSeparator();
  QAction *quit = new QAction("&Quit", this);
  game->addAction(quit);
  // TODO: Read and understand http://doc.qt.io/archives/qt-4.8/signalsandslots.html
  //connect(new_game, SIGNAL(triggered()), this, SLOT(&MainWindow::new_game()));
  //connect(undo_move, &QAction::triggered, this, MainWindow::undo_move);
  //connect(add_ai, &QAction::triggered, this, MainWindow::add_ai);
  //connect(remove_ai, &QAction::triggered, this, MainWindow::remove_ai);
  connect(quit, &QAction::triggered, qApp, QApplication::quit);
}

void MainWindow::new_game()
{
    qApp->quit();
}

void MainWindow::add_ai()
{
}

void MainWindow::remove_ai()
{
}

void MainWindow::undo_move()
{
}



