include_guard()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_COLOR_DIAGNOSTICS ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)

if (${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS_DEBUG "-ggdb3")
endif()


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
        -Wno-error=unused-but-set-variable
        -Wno-error=unused-function
        -Wno-error=unused-variable
)

function(dbg var)
    message(DEBUG "${var}: ${${var}}")
endfunction()
