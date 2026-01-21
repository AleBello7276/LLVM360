cmake_minimum_required(VERSION 3.30)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_C_COMPILER clang)

project(LLVM360 LANGUAGES CXX)

add_subdirectory(external)
add_subdirectory(llvm360/Naive+)