#pragma once

#include <engine/fundamental.hpp>

#include <vector>

namespace gt
{

enum class attrib
{
    pos,
    uv,
    normal,
};

// sizeof attrib in f32's
// TODO! make in bytes + add various attrib types
constexpr sz attrib_size(attrib a)
{
    using enum attrib;
    switch (a)
    {
    case pos: return 3;
    case uv: return 2;
    case normal: return 3;
    }

    unreachable();
}

constexpr sz attrib_bytesize(attrib a)
{
    return attrib_size(a) * sizeof(f32);
}

struct mesh
{
    std::vector<attrib> attribs;
    std::vector<f32> vertices;
    std::vector<u32> indices;

    // vertex stride == sum of attrib_size
    sz stride() const;
    // vertex stride == sum of attrib_bytesize
    sz bytestride() const;
};

bool from_file(mesh & m, std::filesystem::path const& path);

}
