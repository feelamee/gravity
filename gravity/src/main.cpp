#include "SDL3/SDL_keycode.h"
#include <engine/fundamental.hpp>
#include <engine/context.hpp>
#include <engine/error.hpp>
#include <engine/sdl.hpp>
using namespace gt;

#include <SDL3/SDL.h>

#include <glad/glad.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-conversion"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/io.hpp>
#pragma GCC diagnostic pop

#include <cstdlib>
#include <cmath>
#include <array>
#include <ranges>

constexpr auto vertex_shader_src = R"(
#version 320 es

layout (location = 0) in vec3 position;

layout (location = 1) uniform mat4 model;
layout (location = 2) uniform mat4 view;
layout (location = 3) uniform mat4 projection;

void main()
{
    vec4 pos = vec4(position, 1.0f);
    gl_Position = projection * view * model * pos;
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

template<std::ranges::contiguous_range R>
static GLuint make_vao(
    R const & range,
    size_t elements_per_vertex
)
{
    using value_type = std::ranges::range_value_t<R>;
    static_assert(std::same_as<value_type, f32>);

    GLuint vao, vbo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    glBufferData(
        GL_ARRAY_BUFFER,
        static_cast<GLsizeiptr>(std::ranges::size(range) * sizeof(value_type)),
        std::ranges::data(range),
        GL_STATIC_DRAW
    );
    glVertexAttribPointer(
        0,
        static_cast<GLint>(elements_per_vertex),
        GL_FLOAT,
        GL_FALSE,
        static_cast<GLsizei>(elements_per_vertex * sizeof(value_type)),
        nullptr
    );
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return vao;
}

int main()
{
    context ctx;

    GLuint shader_program = glCreateProgram();
    attach_shader(shader_program, GL_FRAGMENT_SHADER, fragment_shader_src);
    attach_shader(shader_program, GL_VERTEX_SHADER, vertex_shader_src);
    glLinkProgram(shader_program);

    std::array vertices = {
    //    x      y
        -0.5f, -0.5f, 0.0f,
         0.5f, -0.5f, 0.0f,
         0.0f,  0.5f, 0.0f,
    };
    GLuint vao = make_vao(vertices, 3);

    vec3 pos{ 0.0f, 0.0f, 0.0f };
    f32 scaling{ 1.0f };
    f32 rotation{ 0.0f };

    for(;;)
    {
        SDL_Event ev;
        while (SDL_PollEvent(&ev))
        {
            switch (ev.type)
            {
                case SDL_EVENT_QUIT: return EXIT_SUCCESS;
                case SDL_EVENT_KEY_DOWN:
                {
                    switch (ev.key.key)
                    {
                        case SDLK_W: pos.y += 0.1f; break;
                        case SDLK_S: pos.y -= 0.1f; break;
                        case SDLK_A: pos.x -= 0.1f; break;
                        case SDLK_D: pos.x += 0.1f; break;
                        case SDLK_Z: pos.z -= 0.1f; break;
                        case SDLK_X: pos.z += 0.1f; break;
                        case SDLK_Q: rotation -= 0.1f; break;
                        case SDLK_E: rotation += 0.1f; break;
                        case SDLK_EQUALS: scaling += 0.1f; break;
                        case SDLK_MINUS: scaling -= 0.1f; break;
                    }
                }
            }
        }

        glUseProgram(shader_program);

        auto const g = std::fabs(std::sin(f32(SDL_GetTicks()) / 3000.0f));
        auto const b = 1 - std::fabs(std::sin(f32(SDL_GetTicks()) / 3000.0f));
        glUniform4f(0, 0, g, b, 1);

        mat4 model = scale(
            rotate(
                translate( mat4{ 1.0f }, pos),
                rotation,
                vec3{ 0.0f, 0.0f, -1.0f }
            ),
            vec3{ scaling }
        );
        mat4 view = lookAt(
            vec3{0.0f, 0.0f, -1.0f},
           	vec3{0.0f, 0.0f, 0.0f},
            vec3{0.0f, 1.0f, 0.0f}
        );
        mat4 projection = perspective(radians(45.0f), 960.0f / 540.0f, 0.01f, 100.0f);
        glUniformMatrix4fv(1, 1, GL_FALSE, value_ptr(model));
        glUniformMatrix4fv(2, 1, GL_FALSE, value_ptr(view));
        glUniformMatrix4fv(3, 1, GL_FALSE, value_ptr(projection));

        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT);

        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        if (!SDL_GL_SwapWindow(ctx.window))
            sdl::log_error();
    }

    return EXIT_SUCCESS;
}
