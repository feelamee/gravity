#pragma once

#include <engine/util.hpp>

#include <SDL3/SDL.h>

#include <format>

namespace gt
{

struct error : std::runtime_error
{
private:
    using base_t = runtime_error;

public:
    using base_t::base_t;

    template<typename... Args>
    error(std::format_string<Args...> fmt, Args&&... args)
        : base_t(std::format(std::move(fmt), std::forward<Args>(args)...))
    {
    }
};

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
