#pragma once

#include <engine/fundamental.hpp>
#include <engine/math.hpp>

#include <optional>
#include <filesystem>
#include <iosfwd>

namespace gt
{

// Represents image data in memory
// Data stored in row-major format - row by row, from up to down.
// Row is pixels from left to right.
// Each pixel is represented as three bytes - R, G and B, each by one byte.
struct image
{
    static std::optional<image> from_file(std::filesystem::path const& path);

    std::vector<u8> data;
    uvec2 size;
};

void dump(std::ostream & out, image const& img);

}
