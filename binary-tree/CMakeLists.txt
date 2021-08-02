cmake_minimum_required(VERSION 3.10)

project("Binary tree")

set(CMAKE_CXX_STANDARD 17)
enable_testing()

add_executable(BinaryTree binary-tree.cpp)
add_test(NAME Tests COMMAND BinaryTree)
