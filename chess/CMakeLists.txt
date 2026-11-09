cmake_minimum_required(VERSION 3.16)
project(chess CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)  # for clangd and clang-tidy

option(CHESS_BUILD_QT "Build the Qt6 GUI" ON)
option(CHESS_BUILD_TESTS "Build the tests (GoogleTest)" ON)
set(CHESS_SANITIZE "" CACHE STRING "Sanitizers, e.g. address,undefined")

find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
  set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
endif()

add_compile_options(-Wall -Wextra)
# Let the compiler use the POPCNT instruction for __builtin_popcountll
# (bit_matrix::get_num_bits_set); without it, counting bits is a call to a
# software routine. Every x86-64 CPU since about 2008 has POPCNT.
add_compile_options(-mpopcnt)
if(CHESS_SANITIZE)
  add_compile_options(-fsanitize=${CHESS_SANITIZE} -fno-omit-frame-pointer)
  add_link_options(-fsanitize=${CHESS_SANITIZE})
endif()

# Engine without GUI dependencies
add_library(chess_engine STATIC
  datastructures/board.cpp
  engine/draw_rules.cpp
  engine/engine.cpp
  engine/evaluation.cpp
  engine/move_generation.cpp
  engine/notation.cpp
  engine/perft.cpp
  engine/scoring.cpp
  ai/random/random_ai.cpp
  ai/naive/naive_ai.cpp
  ai/minimax/minimax_ai.cpp
  ai/alphabeta/alphabeta_ai.cpp
  ai/ai_registry.cpp
  ai/match.cpp)
target_include_directories(chess_engine PUBLIC datastructures)

# Speed measurement (run in a release build), see bench/chess_bench.cpp
add_executable(chess_bench bench/chess_bench.cpp)
target_link_libraries(chess_bench PRIVATE chess_engine)

# Memory measurement (exact heap numbers), see bench/chess_mem.cpp
add_executable(chess_mem bench/chess_mem.cpp)
target_link_libraries(chess_mem PRIVATE chess_engine)

# Long run of all AIs against each other (crashes, strength, speed, memory),
# see bench/chess_arena.cpp
add_executable(chess_arena bench/chess_arena.cpp)
target_link_libraries(chess_arena PRIVATE chess_engine)

if(CHESS_BUILD_QT)
  find_package(Qt6 REQUIRED COMPONENTS Widgets)

  # GUI classes as a library, used by chess_qt and by the GUI tests.
  add_library(chess_qtgui STATIC
    qtgui/MainWindow.cpp
    qtgui/MainWindow.hpp
    qtgui/ChessBoardWidget.cpp
    qtgui/ChessBoardWidget.hpp
    qtgui/SidePanel.cpp
    qtgui/SidePanel.hpp
    qtgui/SimulationDialog.cpp
    qtgui/SimulationDialog.hpp
    qtgui/Theme.cpp
    qtgui/Theme.hpp)
  set_target_properties(chess_qtgui PROPERTIES AUTOMOC ON)
  target_link_libraries(chess_qtgui PUBLIC chess_engine Qt6::Widgets)

  # Piece images (SVG) and their license are compiled in (Qt resource
  # system), so chess_qt runs from anywhere. In code: ":/images/white_k.svg"
  # etc. For a static library Qt adds the resources as an object library that
  # every user links in. Reading SVG needs Qt's SVG image format plugin at
  # runtime (Debian package qt6-svg-plugins), not the QtSvg development files.
  set(CHESS_PIECE_IMAGES "qtgui/images/PIECES-LICENSE.txt")
  foreach(color white black)
    foreach(piece k q r b n p)
      list(APPEND CHESS_PIECE_IMAGES "qtgui/images/${color}_${piece}.svg")
    endforeach()
  endforeach()
  qt_add_resources(chess_qtgui "piece_images"
    PREFIX "/"
    BASE "qtgui"
    FILES ${CHESS_PIECE_IMAGES})

  add_executable(chess_qt qtgui/MainApp.cpp)
  target_link_libraries(chess_qt PRIVATE chess_qtgui)
endif()

# Tests: build with the presets, run with "ctest --preset debug".
# Deep perft depths only run with CHESS_SLOW_TESTS=1 in the environment.
if(CHESS_BUILD_TESTS)
  enable_testing()
  find_package(GTest REQUIRED)
  include(GoogleTest)
  add_executable(chess_tests
    tests/ai_registry_test.cpp
    tests/alphabeta_test.cpp
    tests/attack_tables_test.cpp
    tests/bit_matrix_test.cpp
    tests/bits_test.cpp
    tests/board_test.cpp
    tests/castling_test.cpp
    tests/check_test.cpp
    tests/draw_test.cpp
    tests/evaluation_test.cpp
    tests/fen_fuzz_test.cpp
    tests/fen_test.cpp
    tests/match_test.cpp
    tests/minimax_test.cpp
    tests/memory_layout_test.cpp
    tests/move_validation_test.cpp
    tests/naive_ai_test.cpp
    tests/notation_test.cpp
    tests/perft_test.cpp
    tests/random_ai_test.cpp
    tests/scoring_test.cpp
    tests/symmetry_test.cpp)
  target_link_libraries(chess_tests PRIVATE chess_engine GTest::gtest_main)
  # TIMEOUT: a test that hangs (e.g. on a dialog that waits for a click)
  # fails after 60 s instead of blocking ctest forever.
  gtest_discover_tests(chess_tests PROPERTIES TIMEOUT 60)

  if(CHESS_BUILD_QT)
    find_package(Qt6 REQUIRED COMPONENTS Test)
    add_executable(chess_gui_tests tests/gui_test.cpp)
    target_link_libraries(chess_gui_tests PRIVATE chess_qtgui Qt6::Test GTest::gtest)
    # Runs without a screen (Qt "offscreen" platform), also on a server.
    gtest_discover_tests(chess_gui_tests
      PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen" TIMEOUT 60
      DISCOVERY_TIMEOUT 30)
  endif()
endif()
