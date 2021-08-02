cmake_minimum_required(VERSION 3.10)

project("Stable selection sort")

set(CMAKE_CXX_STANDARD 17)
enable_testing()

add_executable(StableSelectionSort stable-selection-sort.cpp)
add_test(NAME Tests COMMAND StableSelectionSort)
