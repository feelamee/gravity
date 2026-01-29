#include <SDL3/SDL.h>

#include <engine/context.hpp>

#include <glad/glad.h>

#include <cstdlib>
#include <cmath>

int main()
{
    gt::context ctx;

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
                                ctx.window,
                                SDL_GetWindowFlags(ctx.window) & SDL_WINDOW_FULLSCREEN
                            );
                            break;
                    }
            }
        }

        auto const g = std::fabs(std::sin(float(SDL_GetTicks()) / 3000.0f));
        auto const b = 1 - std::fabs(std::sin(float(SDL_GetTicks()) / 3000.0f));
        GT_GL_CHECK(glClearColor(0, g, b, 1));
        GT_GL_CHECK(glClear(GL_COLOR_BUFFER_BIT));
        SDL_GL_SwapWindow(ctx.window);
    }

    return EXIT_SUCCESS;
}
