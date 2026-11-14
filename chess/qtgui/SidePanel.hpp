#pragma once

#include <QWidget>

#include "../datastructures/enums.hpp"

class ChessBoardWidget;
class QComboBox;
class QPushButton;
class QLabel;
class QSpinBox;
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
    QLabel* check = nullptr;  // "Check" while this player is in check
    // "Next move": AI against AI step by step, on the card of the AI to move.
    QPushButton* next = nullptr;
    QComboBox* player = nullptr;  // Human or one of the AIs
    QLabel* depthLabel = nullptr;
    QSpinBox* depth = nullptr;  // only for AIs that search
  };
  QWidget* createCard(piece_color color, PlayerCard& card);
  void refresh();
  void refreshCard(piece_color color, PlayerCard& card);
  // Shows on the card who plays (also after a choice in the Players menu).
  void refreshPlayer(piece_color color, PlayerCard& card);
  // The user chose a player or a depth on the card.
  void choosePlayer(piece_color color, PlayerCard& card);
  void refreshMoves();
  // The result card below the moves: shown when the game is over.
  QWidget* createResultCard();
  void refreshResult();

  ChessBoardWidget* board_;
  PlayerCard white_;
  PlayerCard black_;
  QTableWidget* moves_;
  QWidget* result_ = nullptr;
  QLabel* resultTitle_ = nullptr;   // "White wins", "Draw"
  QLabel* resultReason_ = nullptr;  // "by checkmate", "50-move rule"
};
