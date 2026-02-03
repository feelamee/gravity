#include <engine/fundamental.hpp>
#include <engine/context.hpp>
#include <engine/error.hpp>
#include <engine/sdl.hpp>
#include <engine/ranges.hpp>
#include <engine/mesh.hpp>
#include <engine/image.hpp>
#include <engine/camera.hpp>
using namespace gt;

#include <SDL3/SDL.h>

#include <glad/glad.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-conversion"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/io.hpp>
#pragma GCC diagnostic pop

#include <cstdlib>
#include <vector>
#include <fstream>

constexpr auto vertex_shader_src = R"(
#version 320 es

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 uv;

layout (location = 2) uniform mat4 model;
layout (location = 3) uniform mat4 view;
layout (location = 4) uniform mat4 projection;

layout (location = 5) out vec2 out_uv;

void main()
{
    vec4 pos = vec4(position, 1.0f);
    gl_Position = projection * view * model * pos;
    out_uv = vec2(uv.x, 1.0f - uv.y);
}
)";

constexpr auto fragment_shader_src = R"(
#version 320 es
precision mediump float;

layout (location = 5) in vec2 uv;

layout (location = 6) uniform sampler2D tex;

out vec4 out_color;

void main()
{
    out_color = texture(tex, uv);
}
)";

static void attach_shader(
    GLuint & program,
    GLenum type,
    std::string_view src
)
{
    auto data = src.data();
    auto const size{ GLint(src.size()) };

    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &data, &size);
    glCompileShader(shader);

    {
        GLint success;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            GLchar log[1024];
            glGetShaderInfoLog(shader, 1024, nullptr, log);
            SDL_Log("[ERROR][OpenGL] shader compilation error: %s\n", log);
        }
    }

    glAttachShader(program, shader);
    glDeleteShader(shader);
}

static GLuint make_vao(
    mesh const& mesh,
    std::vector<u32> const& attrs ///< each attr is count of elements
)
{
    GLuint vao, vbo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);

    GLuint ebo;
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER,
        GLsizeiptr(mesh.indices.size() * sizeof(mesh.indices[0])),
        mesh.indices.data(),
        GL_STATIC_DRAW
    );

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(
        GL_ARRAY_BUFFER,
        GLsizeiptr(mesh.vertices.size() * sizeof(mesh.vertices[0])),
        mesh.vertices.data(),
        GL_STATIC_DRAW
    );

    size_t offset = 0;
    for (auto [i, attr] : vs::enumerate(attrs))
    {
        glVertexAttribPointer(
            GLuint(i),
            GLint(attr),
            GL_FLOAT,
            GL_FALSE,
            GLsizei(sizeof(vertex)),
            (void*)offset
        );
        offset += attr * sizeof(f32);

        glEnableVertexAttribArray(GLuint(i));
    }

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    return vao;
}

static GLuint make_tex(image const& img)
{
    GLuint tex;
    glGenTextures(1, &tex);

    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(
        GL_TEXTURE_2D, 0, GL_RGB,
        GLsizei(img.size.x), GLsizei(img.size.y), 0, GL_RGB, GL_UNSIGNED_BYTE, img.data.data()
    );
    glGenerateMipmap(GL_TEXTURE_2D);

    return tex;
}

int main()
{
    context ctx;
    glEnable(GL_DEPTH_TEST);

    GLuint shader_program = glCreateProgram();
    attach_shader(shader_program, GL_FRAGMENT_SHADER, fragment_shader_src);
    attach_shader(shader_program, GL_VERTEX_SHADER, vertex_shader_src);
    glLinkProgram(shader_program);
    {
        // for some reason linkage errors is not passed into glDebugMessageCallback
        GLint success;
        glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
        if (!success)
        {
            GLchar log[1024];
            glGetProgramInfoLog(shader_program, 1024, nullptr, log);
            SDL_Log("[ERROR][OpenGL] program linkage error: %s\n", log);
        }
    }

    std::string_view const model_filepath{ "/home/missed/code/gravity/assets/sasuke.obj" };
    std::string_view const texture_filepath{ "/home/missed/code/gravity/assets/sasuke.ppm" };

    auto mesh = mesh::from_file(model_filepath);
    if (!mesh)
        throw error{ "[ERROR][ENGINE] can't load mesh: {}", model_filepath };

    GLuint vao = make_vao(*mesh, { 3, 2 });

    auto image = image::from_file(texture_filepath);
    if (!image)
        throw error{ "[ERROR][ENGINE] can't load image: {}", texture_filepath };

    GLuint tex = make_tex(*image);

    vec3 pos{ 0.0f, 0.0f, 0.0f };
    f32 scaling{ 1.0f };
    f32 rotation{ 0.0f };

    camera cam;

    f32 delta_time{ 0 };
    f32 last_ticks{ 0 };

    for(;;)
    {
        SDL_Event ev;
        while (SDL_PollEvent(&ev))
        {
            cam.handle_event(ev);

            switch (ev.type)
            {
                case SDL_EVENT_QUIT: return EXIT_SUCCESS;
                case SDL_EVENT_WINDOW_RESIZED:
                    glViewport(0, 0, ev.window.data1, ev.window.data2);
                    break;
            }
        }

        {
            cam.simulate(delta_time);
        }

        {
            glUseProgram(shader_program);

            mat4 model = scale(
                rotate(
                    translate( mat4{ 1.0f }, pos),
                    rotation,
                    vec3{ 0.0f, 0.0f, -1.0f }
                ),
                vec3{ scaling }
            );
            mat4 projection = perspective(radians(45.0f), 960.0f / 540.0f, 0.01f, 100.0f);
            glUniformMatrix4fv(2, 1, GL_FALSE, value_ptr(model));
            glUniformMatrix4fv(3, 1, GL_FALSE, value_ptr(cam.view()));
            glUniformMatrix4fv(4, 1, GL_FALSE, value_ptr(projection));

            glClearColor(0, 0, 0, 1);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            glBindVertexArray(vao);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, tex);
            glDrawElements(GL_TRIANGLES, GLsizei(mesh->indices.size()), GL_UNSIGNED_INT, nullptr);

            if (!SDL_GL_SwapWindow(ctx.window))
                sdl::log_error();
        }

        {
            SDL_Time ticks{};
            SDL_GetCurrentTime(&ticks);
            f32 fticks = f32(ticks % 360'000'000'000) / 1E8f;

            delta_time = fticks - last_ticks;
            last_ticks = fticks;
        }
    }

    return EXIT_SUCCESS;
}
