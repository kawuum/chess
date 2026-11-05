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
if(CHESS_SANITIZE)
  add_compile_options(-fsanitize=${CHESS_SANITIZE} -fno-omit-frame-pointer)
  add_link_options(-fsanitize=${CHESS_SANITIZE})
endif()

# Engine without GUI dependencies
add_library(chess_engine STATIC
  datastructures/board.cpp
  engine/engine.cpp
  engine/move_generation.cpp
  engine/perft.cpp
  ai/random/random_ai.cpp
  ai/ai_registry.cpp)
target_include_directories(chess_engine PUBLIC datastructures)

# Speed measurement (run in a release build), see bench/chess_bench.cpp
add_executable(chess_bench bench/chess_bench.cpp)
target_link_libraries(chess_bench PRIVATE chess_engine)

# Memory measurement (exact heap numbers), see bench/chess_mem.cpp
add_executable(chess_mem bench/chess_mem.cpp)
target_link_libraries(chess_mem PRIVATE chess_engine)

if(CHESS_BUILD_QT)
  find_package(Qt6 REQUIRED COMPONENTS Widgets)

  # GUI classes as a library, used by chess_qt and by the GUI tests.
  add_library(chess_qtgui STATIC
    qtgui/MainWindow.cpp
    qtgui/MainWindow.hpp
    qtgui/ChessBoardWidget.cpp
    qtgui/ChessBoardWidget.hpp)
  set_target_properties(chess_qtgui PROPERTIES AUTOMOC ON)
  target_link_libraries(chess_qtgui PUBLIC chess_engine Qt6::Widgets)

  # Piece images are compiled in (Qt resource system), so chess_qt runs from
  # anywhere. In code: ":/images/white_k.png" etc. For a static library Qt
  # adds the resources as an object library that every user links in.
  set(CHESS_PIECE_IMAGES "")
  foreach(color white black)
    foreach(piece k q r b n p)
      list(APPEND CHESS_PIECE_IMAGES "qtgui/images/${color}_${piece}.png")
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
    tests/bit_matrix_test.cpp
    tests/board_test.cpp
    tests/castling_test.cpp
    tests/check_test.cpp
    tests/fen_test.cpp
    tests/memory_layout_test.cpp
    tests/move_validation_test.cpp
    tests/perft_test.cpp
    tests/random_ai_test.cpp)
  target_link_libraries(chess_tests PRIVATE chess_engine GTest::gtest_main)
  gtest_discover_tests(chess_tests)

  if(CHESS_BUILD_QT)
    find_package(Qt6 REQUIRED COMPONENTS Test)
    add_executable(chess_gui_tests tests/gui_test.cpp)
    target_link_libraries(chess_gui_tests PRIVATE chess_qtgui Qt6::Test GTest::gtest)
    # Runs without a screen (Qt "offscreen" platform), also on a server.
    gtest_discover_tests(chess_gui_tests
      PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
      DISCOVERY_TIMEOUT 30)
  endif()
endif()
