#pragma once

#include <QDialog>
#include <QElapsedTimer>
#include <QString>
#include <array>
#include <atomic>
#include <memory>
#include <vector>

#include "../ai/match.hpp"

class QCheckBox;
class QComboBox;
class QGroupBox;
class QLabel;
class QProgressBar;
class QPushButton;
class QSpinBox;
class QThread;
class ResultBar;

// Plays many games between two AIs without a board and shows the results.
//
// The games run in a thread of their own, so the window stays usable. That
// thread never touches a widget (Qt allows that only in the GUI thread): it
// hands its results to the GUI thread with QMetaObject::invokeMethod, which
// queues a function call in the GUI thread's event loop.
class SimulationDialog : public QDialog {
  Q_OBJECT

 public:
  // ais: the AIs to choose from (tests can add their own).
  explicit SimulationDialog(QWidget* parent = nullptr,
                            std::vector<ai_info> ais = available_ais());
  // Stops a running simulation and waits for its thread.
  ~SimulationDialog() override;

  bool isRunning() const { return running_; }
  // The results so far (or of the last simulation) as text.
  QString resultText() const;

  // Half moves after which a game counts as unfinished.
  static constexpr int kMaxPlies = 500;

 public slots:
  void start();
  void stop();

 signals:
  // A simulation has ended: all games played, or stopped.
  void simulationEnded();

 protected:
  void closeEvent(QCloseEvent* event) override;

 private:
  // The controls for one of the two AIs: which AI, and its seed. The seed
  // row is only shown for an AI that uses random numbers. "Random seed"
  // draws a new seed at every start (and greys out the field).
  struct AiChooser {
    QComboBox* ai = nullptr;
    QWidget* seedRow = nullptr;
    QCheckBox* randomSeed = nullptr;
    QSpinBox* seed = nullptr;
  };
  // name: "first" or "second", the start of the object names (for tests).
  QGroupBox* createChooser(const QString& title, const QString& name,
                           AiChooser& chooser);
  const ai_info& selectedAi(const AiChooser& chooser) const;
  // Shows or hides the seed row to match the selected AI.
  void updateSeedRow(AiChooser& chooser);
  // The seed to play with. A random seed is drawn now and written into the
  // field, so the user can see it and play the same games again.
  uint32_t seedFor(AiChooser& chooser);

  // A small card with a caption and a big number, e.g. "Draws" / "57".
  struct Tile {
    QLabel* caption = nullptr;
    QLabel* value = nullptr;
  };
  // name: start of the object names (for tests), e.g. "draws".
  // explanation: the tooltip of the tile.
  QWidget* createTile(Tile& tile, const QString& name,
                      const QString& explanation);

  // Called in the GUI thread (queued by the simulation thread).
  void showProgress(const match_stats& stats);
  // The bar, the numbers and the draw reasons for these results.
  void showVisuals(const match_stats& stats);
  void showEnd(const match_stats& stats);
  // The simulation thread ended with an exception (a bug, e.g. in an AI).
  void showError(const QString& message);
  void setRunning(bool running);
  QString describeStats(const match_stats& stats) const;

  // The thread keeps pointers into this list; it lives as long as the
  // dialog, and the destructor waits for the thread.
  const std::vector<ai_info> ais_;
  AiChooser first_;
  AiChooser second_;
  QSpinBox* games_;
  QCheckBox* swapColors_;
  QPushButton* startButton_;
  QPushButton* stopButton_;
  QProgressBar* progress_;
  QLabel* progressText_;  // "37 / 100 games"
  ResultBar* bar_;
  Tile firstWins_, draws_, secondWins_, speed_;
  std::array<QLabel*, 6> chips_{};  // draw reasons, unfinished, length
  QLabel* result_;                  // all results as text
  QElapsedTimer clock_;             // since start, for games per second

  std::unique_ptr<QThread> thread_;
  // Set by the GUI thread, read by the simulation thread after each game.
  std::atomic<bool> cancel_{false};
  bool running_ = false;
  // About the running (or last) simulation, for the result text.
  QString firstName_, secondName_;
  QString seedsText_;
  int plannedGames_ = 0;
  match_stats lastStats_;  // the latest results shown
};
