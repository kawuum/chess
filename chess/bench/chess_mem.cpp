// chess_mem: memory use of the engine.
//
//   chess_mem                  run all measurements
//   chess_mem --save FILE      run them and save the results as baseline
//   chess_mem --compare FILE   run them and compare with a saved baseline
//
// The global operator new/delete are replaced by versions that count every
// heap allocation. Unlike times, these numbers are exactly reproducible: the
// same code gives the same numbers, so every difference to the baseline is a
// real change (a different compiler or standard library can change them too).
// That is also why this is a separate program: counting would slow down the
// time measurements of chess_bench.
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <map>
#include <memory>
#include <new>
#include <sstream>
#include <string>
#include <vector>

#include "../ai/random/random_ai.hpp"
#include "../engine/perft.hpp"

// --- Counting allocator
// -------------------------------------------------------

namespace {

struct HeapStats {
  uint64_t live = 0;         // bytes allocated right now
  uint64_t peak = 0;         // highest value of live
  uint64_t total = 0;        // bytes allocated in total (also freed ones)
  uint64_t allocations = 0;  // number of operator new calls
};
HeapStats g_heap;

// Every block gets a header that stores its size, so operator delete knows how
// much is freed. 16 bytes keep the user part aligned like malloc's own blocks.
constexpr std::size_t kHeader = 16;

void* counted_alloc(std::size_t n) {
  char* block = static_cast<char*>(std::malloc(n + kHeader));
  if (block == nullptr) throw std::bad_alloc();
  *reinterpret_cast<std::size_t*>(block) = n;
  g_heap.live += n;
  g_heap.total += n;
  ++g_heap.allocations;
  if (g_heap.live > g_heap.peak) g_heap.peak = g_heap.live;
  return block + kHeader;
}

void counted_free(void* p) {
  if (p == nullptr) return;
  char* block = static_cast<char*>(p) - kHeader;
  g_heap.live -= *reinterpret_cast<std::size_t*>(block);
  std::free(block);
}

}  // namespace

// Replacing these four is enough: the other variants (nothrow, sized) call
// them by default.
void* operator new(std::size_t n) { return counted_alloc(n); }
void* operator new[](std::size_t n) { return counted_alloc(n); }
void operator delete(void* p) noexcept { counted_free(p); }
void operator delete[](void* p) noexcept { counted_free(p); }
void operator delete(void* p, std::size_t) noexcept { counted_free(p); }
void operator delete[](void* p, std::size_t) noexcept { counted_free(p); }

// --- Measurements
// -------------------------------------------------------------

namespace {

// Results in a fixed order, as "name value" pairs.
std::vector<std::pair<std::string, uint64_t>> g_results;

void record(const std::string& name, uint64_t value) {
  g_results.emplace_back(name, value);
}

// Measures what happens inside f: allocations, bytes, peak above the start.
template <typename F>
HeapStats measure(F f) {
  HeapStats before = g_heap;
  g_heap.peak = g_heap.live;
  f();
  HeapStats d;
  d.live = g_heap.live - before.live;  // still allocated after f
  d.peak = g_heap.peak - before.live;
  d.total = g_heap.total - before.total;
  d.allocations = g_heap.allocations - before.allocations;
  g_heap.peak = std::max(g_heap.peak, before.peak);
  return d;
}

void measure_sizes() {
  record("sizeof_bit_matrix", sizeof(bit_matrix));
  record("sizeof_piece", sizeof(piece));
  record("sizeof_move", sizeof(move));
  record("sizeof_board", sizeof(board));
  record("sizeof_game_history", sizeof(game_history));
  record("sizeof_engine", sizeof(engine));
}

void measure_many_boards() {
  // One million boards in a vector: the plain data size of a board.
  constexpr int kBoards = 1000000;
  HeapStats s = measure([] {
    std::vector<board> boards(kBoards);
    boards[kBoards / 2].add_piece(piece(KING, piece_color::WHITE), 4, 0);
  });
  record("boards_1M_peak_bytes", s.peak);

  // One million game states as the engine creates them (make_shared): the
  // real cost of storing one position in the history.
  HeapStats h = measure([] {
    std::vector<std::shared_ptr<game_history>> states;
    states.reserve(kBoards);
    for (int i = 0; i < kBoards; ++i) {
      states.push_back(std::make_shared<game_history>());
    }
  });
  record("game_states_1M_peak_bytes", h.peak);
}

void measure_game() {
  // A random game with a fixed seed: always the same moves.
  constexpr int kMaxPlies = 200;
  engine e;
  e.new_game();
  random_ai ai(&e, "bench", 42);
  int plies = 0;
  HeapStats s = measure([&] {
    for (; plies < kMaxPlies && e.get_current_gamestate().result == RUNNING;
         ++plies) {
      e.perform_move(ai.next_move());
    }
  });
  record("game_plies", plies);
  record("game_live_bytes", s.live);
  record("game_live_bytes_per_ply", s.live / plies);
  record("game_allocations", s.allocations);
  record("game_allocated_bytes", s.total);
}

void measure_perft(const std::string& name, const char* fen, int depth) {
  engine e;
  e.new_game(fen);
  HeapStats s = measure([&] { perft(e, depth); });
  record(name + "_allocations", s.allocations);
  record(name + "_allocated_bytes", s.total);
  record(name + "_peak_bytes", s.peak);
}

uint64_t peak_rss_kb() {
  // VmHWM = highest resident memory of this process. Includes libraries and
  // allocator reserves, so it is printed but not compared.
  std::ifstream in("/proc/self/status");
  std::string line;
  while (std::getline(in, line)) {
    if (line.rfind("VmHWM:", 0) == 0) return std::stoull(line.substr(6));
  }
  return 0;
}

// --- Output and baseline
// ------------------------------------------------------

std::map<std::string, uint64_t> read_baseline(const std::string& path) {
  std::ifstream in(path);
  if (!in) {
    std::fprintf(stderr, "cannot read baseline %s\n", path.c_str());
    std::exit(2);
  }
  std::map<std::string, uint64_t> values;
  std::string line;
  while (std::getline(in, line)) {
    if (line.empty() || line[0] == '#') continue;
    std::istringstream fields(line);
    std::string name;
    uint64_t v;
    if (fields >> name >> v) values[name] = v;
  }
  return values;
}

void usage() {
  std::fprintf(stderr, "usage: chess_mem [--save FILE | --compare FILE]\n");
}

}  // namespace

int main(int argc, char** argv) {
  std::vector<std::string> args(argv + 1, argv + argc);
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

  measure_sizes();
  measure_many_boards();
  measure_game();
  measure_perft("perft_start_d4",
                "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", 4);
  measure_perft(
      "perft_kiwipete_d3",
      "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",
      3);

  std::map<std::string, uint64_t> baseline;
  if (!compare.empty()) baseline = read_baseline(compare);
  int changed = 0;
  std::printf("%-30s %14s", "measurement", "value");
  if (!baseline.empty()) std::printf(" %14s %9s", "baseline", "change");
  std::printf("\n");
  for (const auto& r : g_results) {
    std::printf("%-30s %14llu", r.first.c_str(), (unsigned long long)r.second);
    auto b = baseline.find(r.first);
    if (b != baseline.end()) {
      if (b->second == r.second) {
        std::printf(" %14llu %9s", (unsigned long long)b->second, "same");
      } else {
        ++changed;
        double pct =
            b->second ? (double(r.second) / double(b->second) - 1) * 100 : 0;
        std::printf(" %14llu %+8.1f%%", (unsigned long long)b->second, pct);
      }
    }
    std::printf("\n");
  }
  std::printf(
      "\npeak resident memory of this process: %llu KB (not compared)\n",
      (unsigned long long)peak_rss_kb());
  if (!baseline.empty()) {
    std::printf(
        "%d of %zu values differ from the baseline (no noise: every "
        "difference is a real change)\n",
        changed, g_results.size());
  }

  if (!save.empty()) {
    std::ofstream out(save);
    out << "# chess_mem baseline: exact heap numbers, see "
           "bench/chess_mem.cpp\n";
    for (const auto& r : g_results) out << r.first << " " << r.second << "\n";
    std::printf("baseline saved to %s\n", save.c_str());
  }
  return 0;
}
