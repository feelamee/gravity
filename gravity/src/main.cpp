#include <SDL3/SDL.h>

#include <engine/context.hpp>
#include <engine/error.hpp>
#include <engine/sdl.hpp>

#include <glad/glad.h>

#include <cstdlib>
#include <cmath>
#include <array>

constexpr auto vertex_shader_src = R"(
#version 320 es

layout (location = 0) in vec2 position;

void main()
{
    gl_Position = vec4(position, 0.0f, 1.0f);
}
)";

constexpr auto fragment_shader_src = R"(
#version 320 es
precision mediump float;

out vec4 out_color;

layout (location = 0) uniform vec4 color;

void main()
{
    out_color = color;
}
)";

static void attach_shader(
    GLuint & program,
    GLenum type,
    std::string_view src
)
{
    auto data = src.data();
    auto const size{ static_cast<GLint>(src.size()) };

    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &data, &size);
    glCompileShader(shader);
    glAttachShader(program, shader);
    glDeleteShader(shader);
}

int main()
{
    gt::context ctx;

    GLuint shader_program = glCreateProgram();
    attach_shader(shader_program, GL_FRAGMENT_SHADER, fragment_shader_src);
    attach_shader(shader_program, GL_VERTEX_SHADER, vertex_shader_src);
    glLinkProgram(shader_program);

    GLuint vao, vbo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    std::array vertices = {
    //    x      y
        -0.5f, -0.5f,
         0.5f, -0.5f,
         0.0f,  0.5f,
    };
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    for(;;)
    {
        SDL_Event ev;
        while (SDL_PollEvent(&ev))
        {
            switch (ev.type)
            {
                case SDL_EVENT_QUIT: return EXIT_SUCCESS;
            }
        }

        {
            auto const g = std::fabs(std::sin(float(SDL_GetTicks()) / 3000.0f));
            auto const b = 1 - std::fabs(std::sin(float(SDL_GetTicks()) / 3000.0f));
            glUseProgram(shader_program);
            glUniform4f(0, 0, g, b, 1);
            glUseProgram(0);
        }

        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shader_program);
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        if (!SDL_GL_SwapWindow(ctx.window))
            gt::sdl::log_error();
    }

    return EXIT_SUCCESS;
}
