cmake_minimum_required(VERSION 3.10)
project(gravity)


option(GRAVITY_DEV "build for development" OFF)
option(GRAVITY_SANITIZERS "enable sanitizers" ${GRAVITY_DEV})
option(GRAVITY_ASSERTIONS "enable assertions" ${GRAVITY_DEV})
option(GRAVITY_HARDENING "enable hardening" OFF)


list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(common)


add_subdirectory(engine)
add_subdirectory(gravity)

