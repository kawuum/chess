#include "SimulationDialog.hpp"

#include <QCheckBox>
#include <QCloseEvent>
#include <QComboBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>
#include <QSpinBox>
#include <QStringList>
#include <QThread>
#include <QVBoxLayout>
#include <random>

namespace {

// Seeds the user can type, and the range random seeds are drawn from.
constexpr int kMaxSeed = 999999999;

}  // namespace

SimulationDialog::SimulationDialog(QWidget* parent) : QDialog(parent) {
  setWindowTitle("Simulate games");

  games_ = new QSpinBox(this);
  games_->setRange(1, 100000);
  games_->setValue(100);
  games_->setObjectName("games");  // object names let tests find widgets
  auto* form = new QFormLayout;
  form->addRow("Games:", games_);

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

  auto* choosers = new QHBoxLayout;
  choosers->addWidget(createChooser("First AI", "first", first_));
  choosers->addWidget(createChooser("Second AI", "second", second_));

  auto* layout = new QVBoxLayout(this);
  layout->addLayout(choosers);
  layout->addLayout(form);
  layout->addLayout(buttons);
  layout->addWidget(progress_);
  layout->addWidget(result_);

  setRunning(false);
}

QGroupBox* SimulationDialog::createChooser(const QString& title,
                                           const QString& name,
                                           AiChooser& chooser) {
  auto* box = new QGroupBox(title, this);

  chooser.ai = new QComboBox(box);
  for (const ai_info& ai : available_ais()) {
    chooser.ai->addItem(QString::fromStdString(ai.name));
  }
  chooser.ai->setObjectName(name + "Ai");

  chooser.randomSeed = new QCheckBox("Random seed", box);
  chooser.randomSeed->setChecked(true);
  chooser.randomSeed->setObjectName(name + "RandomSeed");
  chooser.seed = new QSpinBox(box);
  chooser.seed->setRange(0, kMaxSeed);
  chooser.seed->setValue(1);
  chooser.seed->setEnabled(false);  // "Random seed" is checked
  chooser.seed->setObjectName(name + "Seed");
  chooser.seed->setToolTip("The same seed plays the same games again.");
  // The checkbox greys out the field: a random seed is drawn at the start.
  connect(chooser.randomSeed, &QCheckBox::toggled, chooser.seed,
          [&chooser](bool random) { chooser.seed->setEnabled(!random); });

  chooser.seedRow = new QWidget(box);
  chooser.seedRow->setObjectName(name + "SeedRow");
  auto* seedLayout = new QHBoxLayout(chooser.seedRow);
  seedLayout->setContentsMargins(0, 0, 0, 0);
  seedLayout->addWidget(new QLabel("Seed:", chooser.seedRow));
  seedLayout->addWidget(chooser.seed, 1);
  seedLayout->addWidget(chooser.randomSeed);

  connect(chooser.ai, &QComboBox::currentIndexChanged, this,
          [this, &chooser] { updateSeedRow(chooser); });
  updateSeedRow(chooser);

  auto* layout = new QVBoxLayout(box);
  layout->addWidget(chooser.ai);
  layout->addWidget(chooser.seedRow);
  layout->addStretch();
  return box;
}

const ai_info& SimulationDialog::selectedAi(const AiChooser& chooser) const {
  return available_ais()[chooser.ai->currentIndex()];
}

void SimulationDialog::updateSeedRow(AiChooser& chooser) {
  chooser.seedRow->setVisible(selectedAi(chooser).uses_random);
}

uint32_t SimulationDialog::seedFor(AiChooser& chooser) {
  if (chooser.randomSeed->isChecked()) {
    std::uniform_int_distribution<int> draw(0, kMaxSeed);
    std::random_device device;
    chooser.seed->setValue(draw(device));
  }
  return static_cast<uint32_t>(chooser.seed->value());
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
  const ai_info* first = &selectedAi(first_);
  const ai_info* second = &selectedAi(second_);
  match_settings settings;
  settings.games = games_->value();
  settings.max_plies = kMaxPlies;
  settings.first_seed = seedFor(first_);
  settings.second_seed = seedFor(second_);

  firstName_ = first_.ai->currentText();
  secondName_ = second_.ai->currentText();
  // Name the seeds of the AIs that use them, so the games can be repeated.
  QStringList seeds;
  if (first->uses_random) {
    seeds << QString("first AI %1").arg(settings.first_seed);
  }
  if (second->uses_random) {
    seeds << QString("second AI %1").arg(settings.second_seed);
  }
  seedsText_ = seeds.isEmpty() ? QString() : "Seeds: " + seeds.join(", ");
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
  games_->setEnabled(!running);
  for (AiChooser* chooser : {&first_, &second_}) {
    chooser->ai->setEnabled(!running);
    chooser->randomSeed->setEnabled(!running);
    chooser->seed->setEnabled(!running && !chooser->randomSeed->isChecked());
  }
}

QString SimulationDialog::describeStats(const match_stats& stats) const {
  QString text = QString::fromStdString(
      describe(stats, firstName_.toStdString(), secondName_.toStdString()));
  if (!seedsText_.isEmpty()) text += seedsText_ + "\n";
  return text;
}

QString SimulationDialog::resultText() const { return result_->text(); }
