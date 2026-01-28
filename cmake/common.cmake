include_guard()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_COLOR_DIAGNOSTICS ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)


add_library(sanitizers INTERFACE)
target_compile_options(
    sanitizers
    INTERFACE
        -fsanitize=address,undefined
)
target_link_options(
    sanitizers
    INTERFACE
        -fsanitize=address,undefined
)


if (${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
    add_library(stdlib_assertions INTERFACE)
    target_compile_options(
        stdlib_assertions
        INTERFACE
            -D_GLIBCXX_ASSERTIONS
    )

    add_library(hardening INTERFACE)
    target_compile_options(hardening INTERFACE -fhardened)
elseif (${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
    add_library(stdlib_assertions INTERFACE)
    target_compile_options(
        stdlib_assertions
        INTERFACE
            -D_LIBCPP_HARDENING_MODE=_LIBCPP_HARDENING_MODE_DEBUG
    )

    add_library(hardening INTERFACE)
    target_compile_options(
        hardening
        INTERFACE
            $<$<NOT:$<CONFIG:Debug>>:-D_FORTIFY_SOURCE=3>
            -D_LIBCPP_HARDENING_MODE=_LIBCPP_HARDENING_MODE_DEBUG
            -ftrivial-auto-var-init=zero
            -fPIE
            -fstack-protector-strong
            -fstack-clash-protection
            -fcf-protection=full
    )
    target_link_options(
        hardening
        INTERFACE
            -pie
            -Wl,-z,now
            -Wl,-z,relro
    )
endif()


add_library(dev_flags INTERFACE)
target_compile_options(
    dev_flags
    INTERFACE
        # TODO add MSVC analogue or check_cxx_compiler_flag at least
        -Wconversion -Wsign-conversion -Wformat=2
        -Wdouble-promotion
        -Wall -Werror -pedantic

        -Wno-error=unused
)


function(add_shaders target)
    set(sources ${ARGN})
    list(LENGTH sources N)
    if (${N} EQUAL 0)
        message(FATAL_ERROR "There are must be at least one shader file in SHADER_SOURCES")
    endif()

    find_program(glslc "glslc" REQUIRED)

    set(outputs)
    foreach(src ${sources})
        cmake_path(ABSOLUTE_PATH src NORMALIZE)
        cmake_path(GET src FILENAME name)

        set(out ${CMAKE_CURRENT_BINARY_DIR}/${name}.spv)

        add_custom_command(
            COMMAND ${glslc} ${src} -o ${out}
            COMMENT "[${target}] compiling shader: ${src}"
            OUTPUT ${out}
            DEPENDS ${src}
            VERBATIM
        )

        list(APPEND outputs ${out})
    endforeach()

    add_custom_target(${target} DEPENDS ${outputs})
endfunction()

function(add_assets target)
    set(sources ${ARGN})
    list(LENGTH sources N)
    if (${N} EQUAL 0)
        message(FATAL_ERROR "There are must be at least one asset file in ASSETS")
    endif()

    find_program(cmake "cmake" REQUIRED)

    set(outputs)
    foreach(src ${ARGN})
        cmake_path(ABSOLUTE_PATH src NORMALIZE)
        cmake_path(
            RELATIVE_PATH src
            BASE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            OUTPUT_VARIABLE relative_src
        )
        set(out ${CMAKE_CURRENT_BINARY_DIR}/${relative_src})

        add_custom_command(
            COMMAND ${cmake} -E copy ${src} ${out}
            COMMENT "[${target}] copying assets: ${src}"
            OUTPUT ${out}
            DEPENDS ${src}
            VERBATIM
        )

        list(APPEND outputs ${out})
    endforeach()

    add_custom_target(${target} DEPENDS ${outputs})
endfunction()

function(dbg var)
    message(DEBUG "${var}: ${${var}}")
endfunction()
