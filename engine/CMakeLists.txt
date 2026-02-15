cmake_minimum_required(VERSION 3.10)
project(gravity::engine LANGUAGES CXX)

add_library(engine)
add_library(engine::engine ALIAS engine)
target_sources(
    engine
    INTERFACE
    FILE_SET public_headers
    TYPE HEADERS
    BASE_DIRS include/
    FILES
		include/engine/fundamental.hpp
		include/engine/util.hpp
		include/engine/context.hpp
		include/engine/error.hpp
		include/engine/sdl.hpp
		include/engine/math.hpp
		include/engine/ranges.hpp
		include/engine/mesh.hpp
		include/engine/image.hpp
		include/engine/camera.hpp
		include/engine/gl.hpp
		include/engine/algorithm.hpp
		include/engine/imgui.hpp

    PRIVATE
    FILE_SET private_headers
    TYPE HEADERS
    BASE_DIRS
        include/
        src/
    FILES
)
target_sources(
    engine
    PRIVATE
        src/util.cpp
        src/context.cpp
        src/sdl.cpp
        src/mesh.cpp
        src/image.cpp
        src/camera.cpp
        src/gl.cpp
        src/imgui.cpp
)
target_compile_features(engine PUBLIC cxx_std_20)

include(CPM)
CPMAddPackage(
  NAME SDL3
  GIT_REPOSITORY "https://github.com/libsdl-org/SDL"
  GIT_TAG release-3.4.0
  OPTIONS "SDL_ASAN ${GRAVITY_SANITIZERS}"
)
CPMAddPackage(
  NAME glm
  GIT_REPOSITORY "https://github.com/g-truc/glm"
  GIT_TAG 1.0.3
  OPTIONS "GLM_ENABLE_CXX_20 ON"
)

target_link_libraries(
    engine
    PUBLIC
        $<$<BOOL:${GRAVITY_DEV}>:dev_flags>
        $<$<BOOL:${GRAVITY_SANITIZERS}>:sanitizers>
        $<$<BOOL:${GRAVITY_ASSERTIONS}>:stdlib_assertions>
        $<$<BOOL:${GRAVITY_HARDENING}>:hardening>
)

# TODO! make PRIVATE after removing all SDL/OpenGL code from public interface
target_link_libraries(engine PUBLIC SDL3::SDL3 glad::glad)

target_link_libraries(engine PUBLIC glm::glm imgui::imgui)

target_precompile_headers(
    engine
    PUBLIC
        <algorithm>
        <cassert>
        <charconv>
        <concepts>
        <cstddef>
        <cstdint>
        <filesystem>
        <memory>
        <ranges>
        <span>
        <stdexcept>
        <string>
        <string_view>
        <type_traits>
        <utility>
        <vector>
        <unordered_map>
        <map>
        <unordered_set>
        <set>

        <SDL3/SDL_events.h>
        <SDL3/SDL_filesystem.h>
        <SDL3/SDL_gpu.h>
        <SDL3/SDL_hints.h>
        <SDL3/SDL_init.h>
        <SDL3/SDL_iostream.h>
        <SDL3/SDL_vulkan.h>

        <engine/fundamental.hpp>
        <engine/util.hpp>
        <engine/math.hpp>
        <engine/ranges.hpp>
)
