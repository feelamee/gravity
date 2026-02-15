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
    std::vector<u8> data;
    uvec2 size;

    void reset();
};

bool from_file(image & img, std::filesystem::path const& path);
void dump(std::ostream & out, image const& img);

struct cubemap
{
    // right, left, top, bottom, front, back
    // same order as in GL_TEXTURE_CUBE_MAP_POSITIVE_X,
    // GL_TEXTURE_CUBE_MAP_NEGATIVE_X, etc
    std::array<image, 6> data;

    void reset();
};

// path is a directory with files named right, left, top, bottom, front, back
// with according extension
bool from_file(cubemap & c, std::filesystem::path const &path);

}
