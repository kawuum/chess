cmake_minimum_required(VERSION 3.6)
project(chess)

set(CMAKE_CXX_STANDARD 11)

find_package(wxWidgets COMPONENTS core base REQUIRED)

include("${wxWidgets_USE_FILE}")

#find_package(Qt5Widgets CONFIG REQUIRED)

set(SOURCE_FILES datastructures/bit_matrix.cpp datastructures/board.cpp gui/MainFrame.cpp gui/MainApp.cpp gui/wxImagePanel.cpp engine/engine.cpp engine/move_generation.cpp ai/random/random_ai.cpp)

include_directories(datastructures)

add_executable(chess ${SOURCE_FILES})

target_link_libraries(chess ${wxWidgets_LIBRARIES})
