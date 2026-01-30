#pragma once

#include <source_location>

namespace gt::sdl
{

void log_error(std::source_location l = std::source_location::current());
void throw_error(std::source_location l = std::source_location::current());

bool set_attr(SDL_GLAttr attr, int value);
bool get_attr(SDL_GLAttr attr, int * value);

}
