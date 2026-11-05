// chess_bench: speed test for the engine, to catch real regressions.
//
//   chess_bench                     run the suite and show the results
//   chess_bench --save FILE         run the suite and save it as baseline
//   chess_bench --compare FILE      run the suite and judge it against a
//                                   baseline: PASS (exit code 0) or FAIL (1)
//   chess_bench perft DEPTH [FEN]   count one position (start position if no
//                                   FEN) and show the time
//   chess_bench divide DEPTH [FEN]  like perft, split by the first move
//
// How it measures: every case does one piece of work (a "run") again and
// again for a fixed time; the number of completed runs is the result. More
// runs = faster. Today all cases are perft runs; later cases for the
// evaluation, the search or simulated games fit in the same way.
//
// How it judges: if the number of runs of any case differs by more than 10%
// from the baseline, the test fails. Slower means a regression. Faster means
// the baseline is out of date: after an intended speedup, save a new one.
// Normal noise on this machine is about 2-4%, so 10% only catches real
// changes. A case with fewer than 50 runs also fails: then one run more or
// less is already 2% or more, and the verdict would be luck.
//
// Only meaningful in a release build. A wrong result always fails (exit
// code 2): fast but wrong is worth nothing.
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <functional>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "../engine/perft.hpp"

namespace {

const char* const kStartFen =
    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

constexpr double kSecondsPerCase = 8.0;     // at most 7 cases: about 1 min
constexpr double kTolerancePercent = 10.0;  // allowed difference in runs
constexpr int kMinRuns = 50;                // fewer runs: case is too slow

// One measured case. run() does the work once and returns a check value that
// must equal expected, e.g. the perft node count. The check makes sure the
// case measures correct work and always the same amount of it.
struct BenchCase {
  std::string name;
  std::function<uint64_t()> run;
  uint64_t expected;
};

// A perft case. The engine is set up once; every run counts from the same
// position (perft takes back all moves it makes).
BenchCase perft_case(const char* name, const char* fen, int depth,
                     uint64_t nodes) {
  auto e = std::make_shared<engine>();
  e->new_game(fen);
  return {name, [e, depth] { return perft(*e, depth); }, nodes};
}

// Each perft case covers different rules: castling, en passant, promotions,
// pins. Reference node counts: see tests/perft_test.cpp.
std::vector<BenchCase> make_suite() {
  return {
      perft_case("start", kStartFen, 4, 197281),
      perft_case("kiwipete",
                 "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w "
                 "KQkq - 0 1",
                 3, 97862),
      perft_case("position3", "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1", 5,
                 674624),
      perft_case(
          "position4",
          "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1", 4,
          422333),
      perft_case("position5",
                 "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8", 3,
                 62379),
      perft_case("position6",
                 "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/"
                 "R4RK1 w - - 0 10",
                 3, 89890),
  };
}

struct Result {
  std::string name;
  int runs;        // completed runs within kSecondsPerCase
  double seconds;  // time the runs took
};

double now_seconds() {
  using clock = std::chrono::steady_clock;
  return std::chrono::duration<double>(clock::now().time_since_epoch()).count();
}

Result run_case(const BenchCase& c) {
  double start = now_seconds();
  int runs = 0;
  while (now_seconds() - start < kSecondsPerCase) {
    uint64_t value = c.run();
    if (value != c.expected) {
      std::fprintf(stderr, "FAIL: wrong result for %s: %llu, expected %llu\n",
                   c.name.c_str(), (unsigned long long)value,
                   (unsigned long long)c.expected);
      std::exit(2);
    }
    ++runs;
  }
  return {c.name, runs, now_seconds() - start};
}

// Baseline file: one line per case "name runs", # starts a comment.
std::map<std::string, int> read_baseline(const std::string& path) {
  std::ifstream in(path);
  std::map<std::string, int> runs;
  std::string line;
  while (std::getline(in, line)) {
    if (line.empty() || line[0] == '#') continue;
    std::istringstream fields(line);
    std::string name;
    int r;
    if (fields >> name >> r) runs[name] = r;
  }
  if (runs.empty()) {
    std::fprintf(stderr, "cannot read baseline %s\n", path.c_str());
    std::exit(2);
  }
  return runs;
}

void save_baseline(const std::string& path,
                   const std::vector<Result>& results) {
  std::ofstream out(path);
  std::time_t t = std::time(nullptr);
  char date[32];
  std::strftime(date, sizeof date, "%Y-%m-%d", std::localtime(&t));
  out << "# chess_bench baseline, " << date << ": completed runs per "
      << kSecondsPerCase << " s\n";
  for (const Result& r : results) out << r.name << " " << r.runs << "\n";
}

int run_suite(const std::string& save_path, const std::string& compare_path) {
#ifndef NDEBUG
  std::printf("WARNING: debug build, the results are not meaningful.\n\n");
#endif
  std::map<std::string, int> baseline;
  if (!compare_path.empty()) baseline = read_baseline(compare_path);

  std::printf("%-10s %6s %9s", "case", "runs", "ms/run");
  if (!baseline.empty()) std::printf(" %9s %8s", "baseline", "change");
  std::printf("\n");

  std::vector<Result> results;
  int slower = 0, faster = 0, missing = 0, too_slow = 0;
  for (const BenchCase& c : make_suite()) {
    Result r = run_case(c);
    results.push_back(r);
    std::printf("%-10s %6d %9.2f", r.name.c_str(), r.runs,
                r.seconds * 1000 / r.runs);
    if (!baseline.empty()) {
      auto base = baseline.find(r.name);
      if (base == baseline.end()) {
        ++missing;
        std::printf("   (not in baseline)  FAIL");
      } else {
        double change = (double(r.runs) / base->second - 1) * 100;
        const char* verdict = "ok";
        if (change < -kTolerancePercent) {
          verdict = "FAIL: slower";
          ++slower;
        } else if (change > kTolerancePercent) {
          verdict = "FAIL: faster";
          ++faster;
        }
        std::printf(" %9d %+7.0f%%  %s", base->second, change, verdict);
      }
    }
    if (r.runs < kMinRuns) {
      ++too_slow;
      std::printf("  FAIL: too few runs");
    }
    std::printf("\n");
    std::fflush(stdout);
  }

  // Checked first and also when saving: a baseline with too few runs would
  // judge by luck.
  if (too_slow > 0) {
    std::printf(
        "\nFAIL: %d case(s) with fewer than %d runs in %.0f s. Make the case "
        "smaller (e.g. a lower depth), otherwise the verdict is luck.\n",
        too_slow, kMinRuns, kSecondsPerCase);
    if (!save_path.empty()) std::printf("Baseline not saved.\n");
    return 1;
  }
  if (!save_path.empty()) {
    save_baseline(save_path, results);
    std::printf("\nbaseline saved to %s\n", save_path.c_str());
  }
  if (baseline.empty()) return 0;

  if (slower > 0 || missing > 0) {
    std::printf(
        "\nFAIL: %d case(s) more than %.0f%% slower than the baseline "
        "(regression) or missing in it.\n",
        slower + missing, kTolerancePercent);
    return 1;
  }
  if (faster > 0) {
    std::printf(
        "\nFAIL: %d case(s) more than %.0f%% faster than the baseline.\n"
        "If the speedup is intended, save a new baseline with --save.\n",
        faster, kTolerancePercent);
    return 1;
  }
  std::printf("\nPASS: all cases within %.0f%% of the baseline.\n",
              kTolerancePercent);
  return 0;
}

int run_perft(int depth, const std::string& fen, bool divide) {
  engine e;
  e.new_game(fen);
  double t0 = now_seconds();
  uint64_t nodes = 0;
  if (divide) {
    for (const auto& entry : perft_divide(e, depth)) {
      std::printf("%s: %llu\n", entry.first.c_str(),
                  (unsigned long long)entry.second);
      nodes += entry.second;
    }
  } else {
    nodes = perft(e, depth);
  }
  double t = now_seconds() - t0;
  std::printf("nodes %llu, time %.3f s, %.0f nodes/s\n",
              (unsigned long long)nodes, t, nodes / t);
  return 0;
}

void usage() {
  std::fprintf(stderr,
               "usage: chess_bench [--save FILE | --compare FILE]\n"
               "       chess_bench perft DEPTH [FEN]\n"
               "       chess_bench divide DEPTH [FEN]\n");
}

}  // namespace

int main(int argc, char** argv) {
  std::vector<std::string> args(argv + 1, argv + argc);
  try {
    if (!args.empty() && (args[0] == "perft" || args[0] == "divide")) {
      if (args.size() < 2) {
        usage();
        return 2;
      }
      int depth = std::stoi(args[1]);
      // The FEN may come as one argument or as several words.
      std::string fen;
      for (size_t i = 2; i < args.size(); ++i) {
        fen += (i > 2 ? " " : "") + args[i];
      }
      return run_perft(depth, fen.empty() ? kStartFen : fen,
                       args[0] == "divide");
    }
    std::string save, compare;
    for (size_t i = 0; i < args.size(); ++i) {
      if (args[i] == "--save" && i + 1 < args.size()) {
        save = args[++i];
      } else if (args[i] == "--compare" && i + 1 < args.size()) {
        compare = args[++i];
      } else {
        usage();
        return 2;
      }
    }
    return run_suite(save, compare);
  } catch (const std::exception& ex) {
    std::fprintf(stderr, "error: %s\n", ex.what());
    return 2;
  }
}
