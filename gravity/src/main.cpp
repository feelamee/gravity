#include <engine/fundamental.hpp>
#include <engine/context.hpp>
#include <engine/error.hpp>
#include <engine/sdl.hpp>
#include <engine/ranges.hpp>
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

constexpr auto vertex_shader_src = R"(
#version 320 es

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 in_color;

layout (location = 2) uniform mat4 model;
layout (location = 3) uniform mat4 view;
layout (location = 4) uniform mat4 projection;

out vec3 color;

void main()
{
    vec4 pos = vec4(position, 1.0f);
    gl_Position = projection * view * model * pos;
    color = in_color;
}
)";

constexpr auto fragment_shader_src = R"(
#version 320 es
precision mediump float;

in vec3 color;

out vec4 out_color;

void main()
{
    out_color = vec4(color, 1.0f);
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
    std::span<f32 const> vertices,
    size_t elements_per_vertex,
    std::vector<u32> const & attrs, ///< each attr is count of elements
    std::optional<std::span<u32>> indices
)
{
    GLuint vao, vbo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);

    if (indices.has_value())
    {
        GLuint ebo;
        glGenBuffers(1, &ebo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(
            GL_ELEMENT_ARRAY_BUFFER,
            GLsizeiptr(indices->size() * sizeof(u32)),
            indices->data(),
            GL_STATIC_DRAW
        );
    }

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(
        GL_ARRAY_BUFFER,
        GLsizeiptr(vertices.size() * sizeof(f32)),
        vertices.data(),
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
            GLsizei(elements_per_vertex * sizeof(f32)),
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

struct camera
{
    camera()
        : camera({ 0.0f, 0, 0 }, { 0.0f, 0, -1 }, { 0.0f, 1, 0 })
    {
    }

    camera(vec3 const& pos, vec3 const& dir, vec3 const& up)
        : init_position{ pos }
        , init_direction{ dir }
        , world_up{ up }
        , position{ pos }
        , direction{ dir }
    {
        update_vectors();
    }


    void handle_event(SDL_Event const& ev)
    {
        SDL_Window * window = gt::ctx().window;

        switch (ev.type)
        {
            case SDL_EVENT_KEY_DOWN:
            {
                switch (ev.key.key)
                {
                case SDLK_W:
                    is_w = true;
                    break;
                case SDLK_S:
                    is_s = true;
                    break;
                case SDLK_A:
                    is_a = true;
                    break;
                case SDLK_D:
                    is_d = true;
                    break;
                case SDLK_EQUALS:
                    position = init_position;
                    yaw = init_yaw;
                    pitch = init_pitch;
                    update_vectors();
                    break;
                }

                is_lshift = ev.key.mod & SDL_KMOD_LSHIFT;

                bool const new_is_lctrl = ev.key.mod & SDL_KMOD_LCTRL;
                if (is_lctrl != new_is_lctrl)
                {
                    int w, h;
                    SDL_GetWindowSize(window, &w, &h);
                    SDL_WarpMouseInWindow(window, f32(w) / 2, f32(h) / 2);
                    is_lctrl = new_is_lctrl;
                }
            }
            break;

            case SDL_EVENT_KEY_UP:
            {
                switch (ev.key.key)
                {
                case SDLK_W:
                    is_w = false;
                    break;
                case SDLK_S:
                    is_s = false;
                    break;
                case SDLK_A:
                    is_a = false;
                    break;
                case SDLK_D:
                    is_d = false;
                    break;
                }

                is_lshift = ev.key.mod & SDL_KMOD_LSHIFT;

                bool const new_is_lctrl = ev.key.mod & SDL_KMOD_LCTRL;
                if (is_lctrl != new_is_lctrl)
                {
                    int w, h;
                    SDL_GetWindowSize(window, &w, &h);
                    SDL_WarpMouseInWindow(window, f32(w) / 2, f32(h) / 2);
                    is_lctrl = new_is_lctrl;
                }
            }
            break;

            case SDL_EVENT_MOUSE_MOTION:
            {
                if (is_lctrl)
                {
                    if (!SDL_SetWindowRelativeMouseMode(window, false))
                        sdl::log_error();
                    SDL_ShowCursor();
                }
                else
                {
                    if (!SDL_SetWindowRelativeMouseMode(window, true))
                        sdl::log_error();
                    SDL_HideCursor();

                    pitch -= ev.motion.yrel * 0.05f;
                    yaw += ev.motion.xrel * 0.05f;

                    pitch = std::clamp(pitch, -89.0f, 89.0f);

                    update_vectors();
                }
            }
            break;
        }
    }

    void simulate(f32 d)
    {
        f32 const real_speed = speed * (is_lshift ? 4.0f : 1.0f);
        if (is_w)
            position += d * real_speed * direction;
        if (is_s)
            position -= d * real_speed * direction;
        if (is_a)
            position -= right * d * real_speed;
        if (is_d)
            position += right * d * real_speed;
    }

    mat4 view() const
    {
        return lookAt(position, position + direction, up);
    }

private:
    void update_vectors()
    {
        direction[0] = std::cos(radians(pitch)) * std::cos(radians(yaw));
        direction[1] = std::sin(radians(pitch));
        direction[2] = std::cos(radians(pitch)) * std::sin(radians(yaw));

        direction = normalize(direction);
        right = normalize(cross(direction, world_up));
        up = normalize(cross(right, direction));
    }

private:
    vec3 init_position;
    vec3 init_direction;
    f32 speed = 0.3f;
    vec3 world_up{};

    vec3 position{};
    vec3 direction{};

    vec3 right{};
    vec3 up{};

    bool is_w{ false };
    bool is_s{ false };
    bool is_a{ false };
    bool is_d{ false };
    bool is_lshift{ false };
    bool is_lctrl{ false };

    f32 init_pitch{ 0 };
    f32 init_yaw{ -90 };

    f32 pitch{ init_pitch };
    f32 yaw{ init_yaw };
};

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

    f32 vertices[] = {
    //     x       y      z      r     g     b
       -0.25f, -0.25f, -0.25f,  0.0f, 1.0f, 0.0f,
        0.25f, -0.25f, -0.25f,  0.0f, 0.0f, 1.0f,
        0.25f,  0.25f, -0.25f,  0.0f, 1.0f, 0.0f,
       -0.25f,  0.25f, -0.25f,  1.0f, 0.0f, 0.0f,
       -0.25f, -0.25f, -0.75f,  1.0f, 0.0f, 0.0f,
        0.25f, -0.25f, -0.75f,  1.0f, 0.0f, 1.0f,
        0.25f,  0.25f, -0.75f,  1.0f, 1.0f, 0.0f,
       -0.25f,  0.25f, -0.75f,  0.0f, 1.0f, 1.0f,
    };
    u32 indices[] = {
        // front
        0, 1, 2,
        0, 2, 3,
        // top
        2, 3, 7,
        2, 7, 6,
        // right
        1, 2, 5,
        2, 5, 6,
        // bottom
        0, 1, 4,
        1, 4, 5,
        // left
        0, 3, 4,
        3, 4, 7,
        // back
        4, 5, 6,
        4, 6, 7,
    };
    size_t const indices_count = sizeof(indices) / sizeof(indices[0]);
    GLuint vao = make_vao(vertices, 6, { 3, 3 }, indices);

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
            mat4 view = cam.view();
            mat4 projection = perspective(radians(45.0f), 960.0f / 540.0f, 0.01f, 100.0f);
            glUniformMatrix4fv(2, 1, GL_FALSE, value_ptr(model));
            glUniformMatrix4fv(3, 1, GL_FALSE, value_ptr(view));
            glUniformMatrix4fv(4, 1, GL_FALSE, value_ptr(projection));

            glClearColor(0, 0, 0, 1);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            glBindVertexArray(vao);
            glDrawElements(GL_TRIANGLES, indices_count, GL_UNSIGNED_INT, nullptr);

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
