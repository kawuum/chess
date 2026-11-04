#pragma once

#include <QMainWindow>

class ChessBoardWidget;

// Main window: game menu, the board in the middle, status bar at the bottom.
class MainWindow : public QMainWindow {
  Q_OBJECT

 public:
  explicit MainWindow(QWidget* parent = nullptr);

 private:
  // Owned by Qt: it is the central widget, so the window deletes it.
  ChessBoardWidget* board_;
};
