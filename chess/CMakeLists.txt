cmake_minimum_required(VERSION 3.6)
project(chess)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.cpp datastructures/bit_matrix.cpp datastructures/board.cpp)

include_directories(datastructures)

add_executable(chess ${SOURCE_FILES})
