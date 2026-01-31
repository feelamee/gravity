#include <engine/context.hpp>

#include <engine/error.hpp>
#include <engine/sdl.hpp>

#include <SDL3/SDL_init.h>
#include <SDL3/SDL_video.h>

#include <glad/glad.h>

namespace gt
{

static void APIENTRY gl_debug_message_callback(
    GLenum source,
    GLenum type,
    GLuint id,
    GLenum severity,
    GLsizei length,
    GLchar const* msg,
    void const*
);

static context * g_context;

context::context()
{
    if (g_context)
        throw error("[ERROR][engine] context must be created only once");

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS))
        sdl::throw_error();

    window = SDL_CreateWindow(
        "gravity simulation",
        960, 540,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
    );
    if (!window)
        sdl::throw_error();

    constexpr auto profile =
#if defined(__WIN32__)
        SDL_GL_CONTEXT_PROFILE_COMPATIBILITY
#elif defined(__ANDROID__)
        SDL_GL_CONTEXT_PROFILE_ES
#else
        SDL_GL_CONTEXT_PROFILE_CORE
#endif
        ;

    sdl::set_attr(SDL_GL_CONTEXT_PROFILE_MASK, profile);
    sdl::set_attr(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    sdl::set_attr(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    sdl::set_attr(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);

    glcontext = SDL_GL_CreateContext(window);
    if (!glcontext)
        sdl::throw_error();

    auto const load_gl_fn = [](char const* fn)
    {
        return reinterpret_cast<void *>(SDL_GL_GetProcAddress(fn));
    };

    int errc = gladLoadGLES2Loader(load_gl_fn);
    if (!errc)
        throw error("[ERROR][engine] gladLoadGLES2Loader failed");

    GLubyte const * real_version = glGetString(GL_VERSION);
    GT_LOG_DEBUG("[INFO] OpenGL is initialized: %s\n", real_version);

    {
        int w{ 0 }, h{ 0 };
        if (!SDL_GetWindowSize(window, &w, &h))
            sdl::throw_error();

        glViewport(0, 0, w, h);
    }

#ifndef NDEBUG
    glDebugMessageCallback(&gl_debug_message_callback, nullptr);
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
#endif

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

void APIENTRY gl_debug_message_callback(
    GLenum source,
    GLenum type,
    GLuint id,
    GLenum severity,
    GLsizei length,
    GLchar const* msg,
    void const*
)
{
    static GLuint last_id = GLuint(-1);
    static unsigned message_strick = 0;
    constexpr unsigned const max_message_strick = 5;

    if (last_id == id)
        ++message_strick;
    else
    {
        last_id = id;
        message_strick = 0;
    }

    if (message_strick == max_message_strick)
    {
        SDL_Log(
            "Last message was repeated %d times. Now it will be suppressed\n",
            max_message_strick
        );
    }

    if (message_strick >= max_message_strick)
        return;

    char const* source_str;
    char const* type_str;
    char const* severity_str;

#define CASE(var, en) case (en): (var) = #en; break

    // TODO! somehow match source/type/severity to SDL log system?
    switch (source)
    {
        CASE(source_str, GL_DEBUG_SOURCE_API);
        CASE(source_str, GL_DEBUG_SOURCE_WINDOW_SYSTEM);
        CASE(source_str, GL_DEBUG_SOURCE_SHADER_COMPILER);
        CASE(source_str, GL_DEBUG_SOURCE_THIRD_PARTY);
        CASE(source_str, GL_DEBUG_SOURCE_APPLICATION);
        CASE(source_str, GL_DEBUG_SOURCE_OTHER);
        default: source_str = "GL_DEBUG_SOURCE_UNKNOWN";
    }

    switch (type) {
        CASE(type_str, GL_DEBUG_TYPE_ERROR);
        CASE(type_str, GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR);
        CASE(type_str, GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR);
        CASE(type_str, GL_DEBUG_TYPE_PORTABILITY);
        CASE(type_str, GL_DEBUG_TYPE_PERFORMANCE);
        CASE(type_str, GL_DEBUG_TYPE_OTHER);
        CASE(type_str, GL_DEBUG_TYPE_MARKER);
        default: type_str = "GL_DEBUG_TYPE_UNKNOWN";
    }

    switch (severity) {
        CASE(severity_str, GL_DEBUG_SEVERITY_HIGH);
        CASE(severity_str, GL_DEBUG_SEVERITY_MEDIUM);
        CASE(severity_str, GL_DEBUG_SEVERITY_LOW);
        CASE(severity_str, GL_DEBUG_SEVERITY_NOTIFICATION);
        default: severity_str = "GL_DEBUG_SEVERITY_UNKNOWN";
    }

#undef CASE

    SDL_Log(
        "[ID %d] [%s] [%s] [%s]:\n    %.*s\n",
        id, type_str, severity_str, source_str, length, msg
    );
}


}
