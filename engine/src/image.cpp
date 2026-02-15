#include <engine/image.hpp>
#include <engine/ranges.hpp>

#include <fstream>

#include <SDL3/SDL_log.h>

namespace gt::ppm
{

using namespace std::string_view_literals;

static bool load(image & r, std::istream & in)
{
    {
        char magic[2];
        in.read(magic, 2);
        if (std::memcmp(magic, "P6", 2) != 0)
            return false;
    }

    u32 max_color_value;
    in >> r.size.x >> r.size.y >> max_color_value;
    if (max_color_value != 255)
        return {};

    std::ignore = in.get();

    r.data.resize(r.size.x * r.size.y * 3);
    in.read(
        reinterpret_cast<char*>(r.data.data()),
        std::streamsize(r.data.size())
    );

    return true;
}

}

namespace gt
{

void image::reset()
{
    data.clear();
    size = {};
}

bool from_file(image & img, std::filesystem::path const& path)
try
{
    img.reset();

    std::ifstream in{ path };
    in.exceptions(std::ifstream::badbit | std::ifstream::failbit);

    // TODO! better use first bytes of file to get filetype (if file is binary)
    auto const ext{ path.extension() };
    if (ext == ".ppm")
        return ppm::load(img, in);

    SDL_Log("[ERROR][engine] can't load image %s: unsupported format %s\n", path.c_str(), ext.c_str());
    return false;
}
catch (std::ifstream::failure const& e)
{
    SDL_Log("[ERROR][engine] can't load image %s: %s", path.c_str(), e.what());
    return false;
}

void dump(std::ostream & out, image const& img)
{
    out << "P6" << '\n';
    out << img.size.x << ' ' << img.size.y << '\n';
    out << 255 << '\n';

    out.write(
        reinterpret_cast<char const*>(img.data.data()),
        std::streamsize(img.data.size())
    );
}

void cubemap::reset()
{
    std::ranges::for_each(data, &image::reset);
}


bool from_file(cubemap & c, std::filesystem::path const& path)
{
    c.reset();

    if (!std::filesystem::is_directory(path))
    {
        SDL_Log("[ERROR][engine] can't load cubemap %s: expected directory", path.c_str());
        return false;
    }

    // TODO! implement compile time map (with mapping to both sides)
    static std::unordered_map<std::string_view, sz> const faces = {
        { "right", 0 },
        { "left", 1},
        { "top", 2 },
        { "bottom", 3 },
        { "front", 4 },
        { "back", 5},
    };
    for (auto const& entry : std::filesystem::directory_iterator{ path })
    {
        auto it = faces.find(entry.path().stem().c_str());
        if (it != end(faces))
        {
            if (!from_file(c.data[it->second], entry.path()))
                return false;
        }
    }

    return rng::none_of(c.data, [](auto const& i) { return i.data.empty(); });
}

}
