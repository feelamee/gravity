#include <engine/gl.hpp>

#include <engine/sdl.hpp>
#include <engine/mesh.hpp>
#include <engine/image.hpp>

#include <fstream>

#include <SDL3/SDL_log.h>

namespace gt::gl
{

detail::resource::operator GLuint() const
{
    return id;
}

void link(shader & s)
{
    glLinkProgram(s);

    // for some reason linkage errors is not passed into glDebugMessageCallback
    GLint success;
    glGetProgramiv(s, GL_LINK_STATUS, &success);
    if (!success)
    {
        static GLchar log[1024];
        glGetProgramInfoLog(s, 1024, nullptr, log);
        SDL_Log("[ERROR][OpenGL] program linkage error: %s\n", log);
    }
}

void attach_source(shader & s, stage stage, std::string_view source)
{
    auto data = (GLchar const*)source.data();
    auto const size{ GLint(source.size()) };

    GLuint shader = glCreateShader(to_underlying(stage));
    glShaderSource(shader, 1, &data, &size);
    glCompileShader(shader);

    if (0)
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

    glAttachShader(s, shader);
    glDeleteShader(shader);
}

void attach_file(shader & s, stage stage, char const* path)
{
    sz size;
    void * data = SDL_LoadFile(path, &size);
    if (!data)
    {
        sdl::log_error();
        return;
    }

    attach_source(s, stage, { (char *)data, size });
    SDL_free(data);
}

static void make_vao(model & m, mesh const& mesh, u32 location)
{
    glGenVertexArrays(1, &m.vao.id);
    glGenBuffers(1, &m.vbo.id);

    glBindVertexArray(m.vao);

    glGenBuffers(1, &m.ebo.id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m.ebo);
    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER,
        GLsizeiptr(mesh.indices.size() * sizeof(mesh.indices[0])),
        mesh.indices.data(),
        GL_STATIC_DRAW
    );

    glBindBuffer(GL_ARRAY_BUFFER, m.vbo);
    glBufferData(
        GL_ARRAY_BUFFER,
        GLsizeiptr(mesh.vertices.size() * sizeof(mesh.vertices[0])),
        mesh.vertices.data(),
        GL_STATIC_DRAW
    );

    sz offset = 0;
    auto attrs = { 3, 2, 3 };
    for (auto [i, attr] : vs::enumerate_with<GLuint>(attrs))
    {
        glVertexAttribPointer(
            location + i,
            GLint(attr),
            GL_FLOAT,
            GL_FALSE,
            GLsizei(sizeof(vertex)),
            (void*)offset
        );
        offset += sz(attr) * sizeof(f32);

        glEnableVertexAttribArray(location + i);
    }

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    m.indices_count = mesh.indices.size();
}

static void make_tex(model & m, image const& img)
{
    texture tex;
    glGenTextures(1, &tex.id);

    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(
        GL_TEXTURE_2D, 0, GL_RGB,
        GLsizei(img.size.x), GLsizei(img.size.y), 0, GL_RGB, GL_UNSIGNED_BYTE, img.data.data()
    );
    glGenerateMipmap(GL_TEXTURE_2D);

    m.textures.push_back(tex);
}

bool from_file(u32 location, model & m, std::filesystem::path const& path)
{
    namespace fs = std::filesystem;
    auto get_path = [&](auto const& s) -> std::optional<fs::path>
    {
        fs::path p{ path.parent_path() / s };

        if (!fs::is_regular_file(p))
            return std::nullopt;

        return p;
    };

    bool has_mesh = false;

    std::ifstream in{ path };
    std::string line;
    while (std::getline(in, line))
    {
        auto pos = line.find(' ');
        if (pos == line.npos)
            continue;

        std::string_view var{ line.data(), pos };
        std::string_view value{ line.data() + pos + 1 };
        if (var == "mesh")
        {
            if (auto p = get_path(value))
                if (auto mesh = mesh::from_file(*p))
                {
                    make_vao(m, *mesh, location);
                    has_mesh = true;
                }
        }
        if (var == "texture")
        {
            if (auto p = get_path(value))
                if (auto img = image::from_file(*p))
                    make_tex(m, *img);
        }
    }

    if (!has_mesh)
    {
        for (texture t : m.textures)
            destroy(t);
    }

    // TODO! if have no textures insert default
    return has_mesh;
}

void bind(shader const& m)
{
    glUseProgram(m);
}

void bind(vertex_array const& vao)
{
    glBindVertexArray(vao);
}

void bind(u32 location, u32 unit, texture const& tex)
{
    glUniform1i(GLint(location), GLint(unit));
    glActiveTexture(GL_TEXTURE0 + GLenum(unit));
    glBindTexture(GL_TEXTURE_2D, tex);
}

void bind(u32 location, model const& m)
{
    bind(m.vao);
    for (auto [i, tex] : vs::enumerate_with<u32>(m.textures))
        bind(location + i, i, tex);
}

void bind(u32 location, mat4 const& m)
{
    glUniformMatrix4fv(GLint(location), 1, GL_FALSE, value_ptr(m));
}

void destroy(shader & s)
{
    glDeleteProgram(s);
}

void destroy(model & m)
{
    destroy(m.vbo);
    destroy(m.ebo);
    destroy(m.vao);
    for (texture t : m.textures)
        destroy(t);
}

void destroy(buffer & b)
{
    glDeleteBuffers(1, &b.id);
}

void destroy(texture & t)
{
    glDeleteTextures(1, &t.id);
}

void destroy(vertex_array & a)
{
    glDeleteVertexArrays(1, &a.id);
}


}
