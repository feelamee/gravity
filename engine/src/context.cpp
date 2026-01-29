#include <engine/context.hpp>

#include <SDL3/SDL_init.h>
#include <SDL3/SDL_video.h>

#include <glad/glad.h>

namespace gt
{

static void throw_sdl_error()
{
    throw error("[ERROR][SDL] {}", SDL_GetError());
}

static context * g_context;

context::context()
{
    if (g_context)
        throw error("[ERROR][engine] context must be created only once");

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS))
        throw_sdl_error();

    window = SDL_CreateWindow(
        "gravity simulation",
        960, 540,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
    );
    if (!window)
        throw_sdl_error();

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

    glcontext = SDL_GL_CreateContext(window);
    if (!glcontext)
        throw_sdl_error();

    auto const load_gl_fn = [](char const* fn)
    {
        return reinterpret_cast<void *>(SDL_GL_GetProcAddress(fn));
    };

    int errc = gladLoadGLES2Loader(load_gl_fn);
    if (!errc)
        throw error("[ERROR][engine] gladLoadGLES2Loader failed");

    {
        int w{ 0 }, h{ 0 };
        if (!SDL_GetWindowSize(window, &w, &h))
            throw_sdl_error();

        GT_GL_CHECK(glViewport(0, 0, w, h));
    }

    g_context = this;
}

context::~context()
{
    SDL_DestroyWindow(window);
    SDL_GL_DestroyContext(glcontext);
    SDL_Quit();
    g_context = nullptr;
}

context & ctx()
{
    if (!g_context)
        throw error("[ERROR][engine] gt::context must be created first");

    return *g_context;
}

}
