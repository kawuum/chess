#pragma once

#include <QMainWindow>
#include <QPointer>

#include "../datastructures/enums.hpp"
#include "SimulationDialog.hpp"

class ChessBoardWidget;
class QMenu;

// Main window: game, player and option menus, the board in the middle, status
// bar at the bottom. Game > Simulate Games opens a SimulationDialog.
class MainWindow : public QMainWindow {
  Q_OBJECT

 public:
  explicit MainWindow(QWidget* parent = nullptr);

  // Text of Help > About, including the license of the piece images.
  static QString aboutText();

 private:
  // Adds a submenu to choose who plays one color: Human or one of the AIs.
  void addPlayerMenu(QMenu* parent, const QString& title, piece_color color);
  // Adds a submenu for the pace of games between two AIs.
  void addAiSpeedMenu(QMenu* parent);
  // Opens the dialog that simulates many games between two AIs.
  void showSimulation();

  // Owned by Qt: it is the central widget, so the window deletes it.
  ChessBoardWidget* board_;
  // Owned by Qt (child of the window); created on first use. QPointer
  // becomes nullptr by itself if the dialog is deleted.
  QPointer<SimulationDialog> simulation_;
};
