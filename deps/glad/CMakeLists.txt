cmake_minimum_required(VERSION 3.12)
project(glad)

add_library(glad STATIC)
target_sources(glad PRIVATE "include/glad/glad.h" "include/KHR/khrplatform.h"
                            "src/glad.c")
target_include_directories(glad PUBLIC "include/")
set_target_properties(glad PROPERTIES POSITION_INDEPENDENT_CODE ON)

add_library(glad::glad ALIAS glad)
