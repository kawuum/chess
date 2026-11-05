#pragma once

#include <QMainWindow>

#include "../datastructures/enums.hpp"

class ChessBoardWidget;
class QMenu;

// Main window: game, player and option menus, the board in the middle, status
// bar at the bottom.
class MainWindow : public QMainWindow {
  Q_OBJECT

 public:
  explicit MainWindow(QWidget* parent = nullptr);

 private:
  // Adds a submenu to choose who plays one color: Human or one of the AIs.
  void addPlayerMenu(QMenu* parent, const QString& title, piece_color color);
  // Adds a submenu for the pace of games between two AIs.
  void addAiSpeedMenu(QMenu* parent);

  // Owned by Qt: it is the central widget, so the window deletes it.
  ChessBoardWidget* board_;
};
