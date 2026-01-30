#include <engine/sdl.hpp>

#include <engine/error.hpp>

#include <SDL3/SDL_log.h>

namespace gt::sdl
{

void log_error(std::source_location l)
{
    SDL_LogError(
        SDL_LOG_CATEGORY_APPLICATION, "[SDL] %s:%d: %s",
        l.file_name(), l.line(), SDL_GetError()
    );
}

void throw_error(std::source_location l)
{
    throw error(
        "[ERROR][SDL] {}:{}: {}",
        l.file_name(), l.line(), SDL_GetError()
    );
}

bool set_attr(SDL_GLAttr attr, int value)
{
    bool r = SDL_GL_SetAttribute(attr, value);
    if (!r)
        sdl::log_error();

    return r;
}

bool get_attr(SDL_GLAttr attr, int * value)
{
    bool r = SDL_GL_GetAttribute(attr, value);
    if (!r)
        sdl::log_error();

    return r;
}

}
