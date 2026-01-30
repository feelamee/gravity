#include <engine/util.hpp>
#include <engine/error.hpp>

#include <glad/glad.h>

#include <SDL3/SDL_log.h>

namespace gt::detail
{

void log_debug(int /*line*/, char const* /*fn*/, char const* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    SDL_LogMessageV(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_DEBUG, fmt, args);
    va_end(args);
}

}
