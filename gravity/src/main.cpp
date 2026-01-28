#include <SDL3/SDL.h>

#include <engine/util.hpp>

#include <format>
#include <cstdlib>
#include <cmath>

static void throw_sdl_error()
{
    throw std::runtime_error(std::format("[ERROR][SDL] {}", SDL_GetError()));
}

int main()
{
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS))
        throw_sdl_error();

    SCOPE_EXIT { SDL_Quit(); };

    SDL_Window * window = SDL_CreateWindow(
        "gravity simulation",
        960, 540,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
    );
    if (!window)
        throw_sdl_error();

    SCOPE_EXIT { SDL_DestroyWindow(window); };

    SDL_Renderer * renderer = SDL_CreateRenderer(window, nullptr);
    if (!renderer)
        throw_sdl_error();

    SCOPE_EXIT { SDL_DestroyRenderer(renderer); };

    for(;;)
    {
        SDL_Event ev;
        while (SDL_PollEvent(&ev))
        {
            switch (ev.type)
            {
                case SDL_EVENT_QUIT: return EXIT_SUCCESS;

                case SDL_EVENT_KEY_UP:
                    switch (ev.key.key)
                    {
                        case SDLK_F:
                            SDL_SetWindowFullscreen(
                                window,
                                SDL_GetWindowFlags(window) & SDL_WINDOW_FULLSCREEN
                            );
                            break;
                    }
            }
        }

        auto const g = Uint8(std::fabs(std::sin(float(SDL_GetTicks()) / 3000.0f)) * 255);
        auto const b = Uint8(255 - std::fabs(std::sin(float(SDL_GetTicks()) / 3000.0f)) * 255);
        SDL_SetRenderDrawColor(renderer, 0, g, b, 255);
        SDL_RenderClear(renderer);
        SDL_RenderPresent(renderer);
    }

    return EXIT_SUCCESS;
}
