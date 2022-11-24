cmake_minimum_required(VERSION 3.10)
project(malcom)

set(CMAKE_CXX_STANDARD 17)

add_library(simulator_lib simulator.cpp)
add_library(dists dists.cpp)
target_link_libraries(simulator_lib dists)
add_executable(simulator_main simulator_main.cpp)
target_link_libraries(simulator_main simulator_lib dists)
