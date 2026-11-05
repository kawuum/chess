#pragma once

#include <QDialog>
#include <QString>
#include <atomic>
#include <memory>

#include "../ai/match.hpp"

class QComboBox;
class QLabel;
class QProgressBar;
class QPushButton;
class QSpinBox;
class QThread;

// Plays many games between two AIs without a board and shows the results.
//
// The games run in a thread of their own, so the window stays usable. That
// thread never touches a widget (Qt allows that only in the GUI thread): it
// hands its results to the GUI thread with QMetaObject::invokeMethod, which
// queues a function call in the GUI thread's event loop.
class SimulationDialog : public QDialog {
  Q_OBJECT

 public:
  explicit SimulationDialog(QWidget* parent = nullptr);
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
  // Called in the GUI thread (queued by the simulation thread).
  void showProgress(const match_stats& stats);
  void showEnd(const match_stats& stats);
  void setRunning(bool running);
  QString describeStats(const match_stats& stats) const;

  QComboBox* firstAi_;
  QComboBox* secondAi_;
  QSpinBox* games_;
  QSpinBox* seed_;
  QPushButton* startButton_;
  QPushButton* stopButton_;
  QProgressBar* progress_;
  QLabel* result_;

  std::unique_ptr<QThread> thread_;
  // Set by the GUI thread, read by the simulation thread after each game.
  std::atomic<bool> cancel_{false};
  bool running_ = false;
  // Names and number of games of the running (or last) simulation.
  QString firstName_, secondName_;
  int plannedGames_ = 0;
};
