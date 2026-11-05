#include "SimulationDialog.hpp"

#include <QCloseEvent>
#include <QComboBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>
#include <QSpinBox>
#include <QThread>
#include <QVBoxLayout>

SimulationDialog::SimulationDialog(QWidget* parent) : QDialog(parent) {
  setWindowTitle("Simulate games");

  firstAi_ = new QComboBox(this);
  secondAi_ = new QComboBox(this);
  for (const ai_info& ai : available_ais()) {
    firstAi_->addItem(QString::fromStdString(ai.name));
    secondAi_->addItem(QString::fromStdString(ai.name));
  }
  games_ = new QSpinBox(this);
  games_->setRange(1, 100000);
  games_->setValue(100);
  seed_ = new QSpinBox(this);
  seed_->setRange(1, 1000000);
  seed_->setValue(1);
  seed_->setToolTip("The same seed plays the same games again.");

  // Object names let the GUI tests find the widgets.
  firstAi_->setObjectName("firstAi");
  secondAi_->setObjectName("secondAi");
  games_->setObjectName("games");

  auto* form = new QFormLayout;
  form->addRow("First AI:", firstAi_);
  form->addRow("Second AI:", secondAi_);
  form->addRow("Games:", games_);
  form->addRow("Seed:", seed_);

  startButton_ = new QPushButton("Start", this);
  stopButton_ = new QPushButton("Stop", this);
  auto* closeButton = new QPushButton("Close", this);
  startButton_->setObjectName("start");
  stopButton_->setObjectName("stop");
  connect(startButton_, &QPushButton::clicked, this, &SimulationDialog::start);
  connect(stopButton_, &QPushButton::clicked, this, &SimulationDialog::stop);
  connect(closeButton, &QPushButton::clicked, this, &QDialog::close);
  auto* buttons = new QHBoxLayout;
  buttons->addWidget(startButton_);
  buttons->addWidget(stopButton_);
  buttons->addStretch();
  buttons->addWidget(closeButton);

  progress_ = new QProgressBar(this);
  result_ = new QLabel(
      QString("Colors alternate from game to game. A game still running "
              "after %1 half moves counts as unfinished.")
          .arg(kMaxPlies),
      this);
  result_->setWordWrap(true);
  result_->setTextInteractionFlags(Qt::TextSelectableByMouse);

  auto* layout = new QVBoxLayout(this);
  layout->addLayout(form);
  layout->addLayout(buttons);
  layout->addWidget(progress_);
  layout->addWidget(result_);

  setRunning(false);
}

SimulationDialog::~SimulationDialog() {
  // The thread uses this dialog (cancel_ and the queued calls), so it must
  // end before the dialog is gone.
  if (thread_) {
    cancel_ = true;
    thread_->wait();
  }
}

void SimulationDialog::start() {
  if (running_) return;
  if (thread_) thread_->wait();  // the last simulation has ended already

  // Pointers into available_ais(): that list lives as long as the program.
  const ai_info* first = &available_ais()[firstAi_->currentIndex()];
  const ai_info* second = &available_ais()[secondAi_->currentIndex()];
  match_settings settings;
  settings.games = games_->value();
  settings.max_plies = kMaxPlies;
  settings.seed = static_cast<uint32_t>(seed_->value());

  firstName_ = firstAi_->currentText();
  secondName_ = secondAi_->currentText();
  plannedGames_ = settings.games;
  progress_->setRange(0, settings.games);
  progress_->setValue(0);
  result_->setText("Running...");
  cancel_ = false;
  setRunning(true);

  // Runs in the new thread. It only reads the AI list (which never changes)
  // and plays on engines of its own; everything for the widgets is queued
  // to the GUI thread.
  auto simulation = [this, first, second, settings] {
    match_stats stats = play_match(
        *first, *second, settings, [this](const match_stats& so_far) {
          QMetaObject::invokeMethod(
              this, [this, so_far] { showProgress(so_far); },
              Qt::QueuedConnection);
          return !cancel_;  // false stops the match
        });
    QMetaObject::invokeMethod(
        this, [this, stats] { showEnd(stats); }, Qt::QueuedConnection);
  };
  thread_.reset(QThread::create(simulation));
  thread_->start();
}

void SimulationDialog::stop() { cancel_ = true; }

void SimulationDialog::closeEvent(QCloseEvent* event) {
  stop();  // no point in playing on for a hidden dialog
  QDialog::closeEvent(event);
}

void SimulationDialog::showProgress(const match_stats& stats) {
  progress_->setValue(stats.games);
  result_->setText(describeStats(stats));
}

void SimulationDialog::showEnd(const match_stats& stats) {
  showProgress(stats);
  if (stats.games < plannedGames_) {
    result_->setText(QString("Stopped after %1 of %2 games.\n\n")
                         .arg(stats.games)
                         .arg(plannedGames_) +
                     describeStats(stats));
  }
  setRunning(false);
  emit simulationEnded();
}

void SimulationDialog::setRunning(bool running) {
  running_ = running;
  startButton_->setEnabled(!running);
  stopButton_->setEnabled(running);
  firstAi_->setEnabled(!running);
  secondAi_->setEnabled(!running);
  games_->setEnabled(!running);
  seed_->setEnabled(!running);
}

QString SimulationDialog::describeStats(const match_stats& stats) const {
  return QString::fromStdString(
      describe(stats, firstName_.toStdString(), secondName_.toStdString()));
}

QString SimulationDialog::resultText() const { return result_->text(); }
