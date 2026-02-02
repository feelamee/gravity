#pragma once

#include <engine/fundamental.hpp>

#include <optional>
#include <string_view>
#include <vector>

namespace gt
{

struct vertex
{
    f32 pos[3];
    f32 uv[2];
    f32 normal[3];

    auto operator<=>(vertex const&) const = default;
};
static_assert(alignof(vertex) == alignof(f32));

struct mesh
{
    static std::optional<mesh> from_file(std::string_view path);

    std::vector<vertex> vertices;
    std::vector<u32> indices;
};

}
