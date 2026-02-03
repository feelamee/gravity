#include <engine/image.hpp>
#include <engine/ranges.hpp>

#include <fstream>

namespace gt
{

namespace
{
namespace ppm
{

using namespace std::string_view_literals;

// from https://en.cppreference.com/w/cpp/numeric/byteswap.html
template<std::integral T>
constexpr T byteswap(T value) noexcept
{
    static_assert(std::has_unique_object_representations_v<T>,
                  "T may not have padding bits");
    auto value_representation = std::bit_cast<std::array<byte, sizeof(T)>>(value);
    std::ranges::reverse(value_representation);
    return std::bit_cast<T>(value_representation);
}


std::optional<image> load(std::istream & in)
{
    {
        char magic[2];
        in.read(magic, 2);
        if (std::memcmp(magic, "P6", 2) != 0)
            return std::nullopt;
    }

    image r;
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

    return r;
}

}
}

std::optional<image> image::from_file(std::filesystem::path const& path)
try
{
    std::ifstream in{ path };
    in.exceptions(std::ifstream::badbit | std::ifstream::failbit);

    // TODO! better use first bytes of file to get filetype
    auto const e{ path.extension() };
    if (e == ".ppm")
        return ppm::load(in);

    return std::nullopt;
}
catch (std::ifstream::failure const& e)
{
    return std::nullopt;
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


}
