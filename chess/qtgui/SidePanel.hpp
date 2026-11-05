#pragma once

#include <QWidget>

#include "../datastructures/enums.hpp"

class ChessBoardWidget;
class QLabel;
class QTableWidget;

// The panel next to the board: a card per player (Black at the top, White at
// the bottom, like on the board) and the moves of the game in between. It
// only reads from the board widget and updates itself after every change.
class SidePanel : public QWidget {
  Q_OBJECT

 public:
  explicit SidePanel(ChessBoardWidget* board, QWidget* parent = nullptr);

 private:
  // One player: a round marker in the player's color, the name, what the
  // player is doing ("your move", "thinking...") and the material lead.
  struct PlayerCard {
    QWidget* frame = nullptr;
    QLabel* marker = nullptr;
    QLabel* name = nullptr;
    QLabel* state = nullptr;
    QLabel* lead = nullptr;
  };
  QWidget* createCard(piece_color color, PlayerCard& card);
  void refresh();
  void refreshCard(piece_color color, PlayerCard& card);
  void refreshMoves();

  ChessBoardWidget* board_;
  PlayerCard white_;
  PlayerCard black_;
  QTableWidget* moves_;
};
