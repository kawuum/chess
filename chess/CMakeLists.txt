cmake_minimum_required(VERSION 3.6)
project(chess)

set(CMAKE_CXX_STANDARD 11)

#find_package(wxWidgets COMPONENTS core base REQUIRED)

#include("${wxWidgets_USE_FILE}")

find_package(Qt5Core)
find_package(Qt5Widgets CONFIG REQUIRED)

set(CMAKE_AUTOMOC ON)

#set(SOURCE_FILES datastructures/bit_matrix.cpp datastructures/board.cpp gui/MainFrame.cpp gui/MainApp.cpp gui/wxImagePanel.cpp engine/engine.cpp engine/move_generation.cpp ai/random/random_ai.cpp)

set(SOURCE_FILES datastructures/bit_matrix.cpp datastructures/board.cpp qtgui/MainWindow.cpp qtgui/MainApp.cpp engine/engine.cpp engine/move_generation.cpp ai/random/random_ai.cpp qtgui/ChessBoardWidget.cpp qtgui/ChessBoardWidget.hpp qtgui/ChessSquare.cpp qtgui/ChessSquare.hpp)

include_directories(datastructures ${Qt5Widgets_INCLUDE_DIRS} ${Qt5Core_INCLUDE_DIRS})

add_executable(chess ${SOURCE_FILES})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${Qt5Widgets_EXECUTABLE_COMPILE_FLAGS} -fPIC")
target_link_libraries(chess Qt5::Widgets Qt5::Core)

#target_link_libraries(${PROJECT} ${wxWidgets_LIBRARIES})
