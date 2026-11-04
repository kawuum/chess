cmake_minimum_required(VERSION 3.16)
project(chess CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)  # für clangd und clang-tidy

option(CHESS_BUILD_QT "Qt6-GUI bauen" ON)
option(CHESS_BUILD_TESTS "Tests bauen (GoogleTest)" ON)
set(CHESS_SANITIZE "" CACHE STRING "Sanitizer, z. B. address,undefined")

find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
  set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
endif()

add_compile_options(-Wall -Wextra)
if(CHESS_SANITIZE)
  add_compile_options(-fsanitize=${CHESS_SANITIZE} -fno-omit-frame-pointer)
  add_link_options(-fsanitize=${CHESS_SANITIZE})
endif()

# Engine ohne GUI-Abhängigkeiten
add_library(chess_engine STATIC
  datastructures/bit_matrix.cpp
  datastructures/board.cpp
  engine/engine.cpp
  engine/move_generation.cpp
  engine/perft.cpp
  ai/random/random_ai.cpp
  ai/ai_registry.cpp)
target_include_directories(chess_engine PUBLIC datastructures)

# Kleines Testprogramm für die Datenstrukturen
add_executable(chess_cli main.cpp)
target_link_libraries(chess_cli PRIVATE chess_engine)

if(CHESS_BUILD_QT)
  find_package(Qt6 REQUIRED COMPONENTS Widgets)
  add_executable(chess_qt
    qtgui/MainApp.cpp
    qtgui/MainWindow.cpp
    qtgui/MainWindow.hpp
    qtgui/ChessBoardWidget.cpp
    qtgui/ChessBoardWidget.hpp)
  set_target_properties(chess_qt PROPERTIES AUTOMOC ON)
  target_link_libraries(chess_qt PRIVATE chess_engine Qt6::Widgets)

  # Piece images are compiled into the binary (Qt resource system), so
  # chess_qt runs from anywhere. In code: ":/images/white_k.png" etc.
  set(CHESS_PIECE_IMAGES "")
  foreach(color white black)
    foreach(piece k q r b n p)
      list(APPEND CHESS_PIECE_IMAGES "qtgui/images/${color}_${piece}.png")
    endforeach()
  endforeach()
  qt_add_resources(chess_qt "piece_images"
    PREFIX "/"
    BASE "qtgui"
    FILES ${CHESS_PIECE_IMAGES})
endif()

# Tests: build with the presets, run with "ctest --preset debug".
# Deep perft depths only run with CHESS_SLOW_TESTS=1 in the environment.
if(CHESS_BUILD_TESTS)
  enable_testing()
  find_package(GTest REQUIRED)
  include(GoogleTest)
  add_executable(chess_tests
    tests/fen_test.cpp
    tests/perft_test.cpp)
  target_link_libraries(chess_tests PRIVATE chess_engine GTest::gtest_main)
  gtest_discover_tests(chess_tests)
endif()
