#include <SDL3/SDL.h>

#include <engine/util.hpp>

#include <glad/glad.h>

#include <format>
#include <cstdlib>
#include <cmath>

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

static void throw_sdl_error()
{
    throw error("[ERROR][SDL] {}", SDL_GetError());
}

int main()
{
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS))
        throw_sdl_error();

    GT_SCOPE_EXIT { SDL_Quit(); };

    SDL_Window * window = SDL_CreateWindow(
        "gravity simulation",
        960, 540,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
    );
    if (!window)
        throw_sdl_error();

    GT_SCOPE_EXIT { SDL_DestroyWindow(window); };

    {
        GLint const desired_major_version{ 3 };
        GLint const desired_minor_version{ 2 };
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, desired_major_version);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, desired_minor_version);

#if defined(__WIN32__)
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
#else
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
#endif

        GLint real_major_version{ 0 };
        SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &real_major_version);
        GLint real_minor_version{ 0 };
        SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &real_minor_version);

        SDL_assert(real_major_version >= desired_major_version);
        SDL_assert(real_minor_version >= desired_minor_version);
    }

    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    if (!gl_context)
        throw_sdl_error();

    GT_SCOPE_EXIT { SDL_GL_DestroyContext(gl_context); };

    auto const load_gl_fn = [](char const* fn)
    {
        return reinterpret_cast<void *>(SDL_GL_GetProcAddress(fn));
    };

    int errc = gladLoadGLES2Loader(load_gl_fn);
    SDL_assert_release(0 != errc);

    {
        int w{ 0 }, h{ 0 };
        if (!SDL_GetWindowSize(window, &w, &h))
            throw_sdl_error();

        GT_GL_CHECK(glViewport(0, 0, w, h));
    }

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

        auto const g = std::fabs(std::sin(float(SDL_GetTicks()) / 3000.0f));
        auto const b = 1 - std::fabs(std::sin(float(SDL_GetTicks()) / 3000.0f));
        GT_GL_CHECK(glClearColor(0, g, b, 1));
        GT_GL_CHECK(glClear(GL_COLOR_BUFFER_BIT));
        SDL_GL_SwapWindow(window);
    }

    return EXIT_SUCCESS;
}
