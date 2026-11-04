cmake_minimum_required(VERSION 3.16)
project(chess CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)  # für clangd und clang-tidy

option(CHESS_BUILD_QT "Qt5-GUI bauen" ON)
option(CHESS_BUILD_WX "wxWidgets-GUI bauen" ON)
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
  ai/random/random_ai.cpp)
target_include_directories(chess_engine PUBLIC datastructures)

# Kleines Testprogramm für die Datenstrukturen
add_executable(chess_cli main.cpp)
target_link_libraries(chess_cli PRIVATE chess_engine)

set(CHESS_IMAGE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/gui/images")

if(CHESS_BUILD_QT)
  find_package(Qt5 REQUIRED COMPONENTS Widgets)
  add_executable(chess_qt
    qtgui/MainApp.cpp
    qtgui/MainWindow.cpp
    qtgui/MainWindow.h
    qtgui/ChessBoardWidget.cpp
    qtgui/ChessSquare.cpp)
  set_target_properties(chess_qt PROPERTIES AUTOMOC ON)
  target_compile_definitions(chess_qt PRIVATE CHESS_IMAGE_DIR="${CHESS_IMAGE_DIR}")
  target_link_libraries(chess_qt PRIVATE chess_engine Qt5::Widgets)
endif()

if(CHESS_BUILD_WX)
  find_package(wxWidgets REQUIRED COMPONENTS core base)
  add_executable(chess_wx
    gui/MainApp.cpp
    gui/MainFrame.cpp
    gui/wxImagePanel.cpp)
  target_compile_definitions(chess_wx PRIVATE CHESS_IMAGE_DIR="${CHESS_IMAGE_DIR}")
  target_link_libraries(chess_wx PRIVATE chess_engine wxWidgets::wxWidgets)
endif()
