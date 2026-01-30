#pragma once

#include <engine/util.hpp>

#include <SDL3/SDL.h>

#include <format>

namespace gt
{

struct context
{
    context(context const&) = delete;
    context& operator=(context const&) = delete;
    context(context &&) = delete;
    context& operator=(context &&) = delete;

    context();
    ~context();

    SDL_Window * window;
    SDL_GLContext glcontext;
};

context & ctx();

}
