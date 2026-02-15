#pragma once

#include <engine/fundamental.hpp>
#include <engine/mesh.hpp>

#include <glad/glad.h>

#include <string_view>

namespace gt::gl
{

namespace detail
{

struct resource
{
    GLuint id;

    operator GLuint() const;
};

}

struct shader : detail::resource {};

enum class stage : GLenum
{
    fragment = GL_FRAGMENT_SHADER,
    vertex = GL_VERTEX_SHADER,
};

void link(shader &);
void attach_source(shader &, stage, std::string_view source);
void attach_file(shader &, stage, std::filesystem::path const& path);

struct buffer : detail::resource {};
struct texture : detail::resource
{
    GLenum type;
};
struct vertex_array : detail::resource {};

struct model
{
    buffer vbo;
    buffer ebo;
    vertex_array vao;
    std::vector<texture> textures;
    sz indices_count;
};

bool from_file(model & m, u32 location, std::filesystem::path const& path);
bool from_file(texture &, GLenum type, std::filesystem::path const& path);

void bind(shader const&);
void bind(vertex_array const&);
void bind(u32 location, u32 unit, texture const&);
void bind(u32 location, model const&);
void bind(u32 location, mat4 const&);

void destroy(shader &);
void destroy(model &);
void destroy(buffer &);
void destroy(texture &);
void destroy(vertex_array &);

}
