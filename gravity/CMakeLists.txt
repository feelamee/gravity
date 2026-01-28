cmake_minimum_required(VERSION 3.10)
project(gravity::gravity LANGUAGES CXX)

add_executable(
    gravity-simulation
    src/main.cpp
)
target_link_libraries(gravity-simulation PRIVATE engine::engine)
