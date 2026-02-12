#include <engine/mesh.hpp>

#include <fstream>
#include <algorithm>
#include <charconv>
#include <strings.h>
#include <unordered_map>
#include <cassert>
#include <filesystem>

#include <SDL3/SDL_log.h>

namespace gt
{

namespace
{

template<typename T>
void hash_combine(sz & seed, T const& v)
{
    using std::hash;
    seed ^= hash<T>{}(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

namespace obj
{

bool isws(char c)
{
    return c <= ' ';
}

char const* ltrim_ws(char const* b, char const* e)
{
    return std::find_if_not(b, e, &isws);
}

char const* ltrim_n(
    char const* b, char const* e,
    char const* chars, sz n
)
{
    std::string_view sv{ b, e };
    auto f = sv.find_first_not_of(chars);
    if (f == sv.npos)
        return b;

    return b + std::min(f, n);
}

template<typename T>
T parse(char const*& b, char const* e, T v)
{
    auto [rest, ec] = std::from_chars(b, e, v);
    b = rest;
    return v;
}

struct index
{
    u32 v, t, n;
};

template<>
index parse<index>(
    char const*& b, char const* e,
    index v
)
{
    v.v = parse(b, e, v.v);
    b = ltrim_n(b, e, "/", 1);
    v.t = parse(b, e, v.t);
    b = ltrim_n(b, e, "/", 1);
    v.n = parse(b, e, v.n);
    return v;
}

index decrement(index v)
{
    assert(v.v > 0);
    assert(v.t > 0);
    assert(v.n > 0);

    v.v -= 1;
    v.t -= 1;
    v.n -= 1;

    return v;
}

template<typename C, typename Proj = std::identity>
void parse_n(
    char const*& b, char const* e,
    size_t n,
    C & out,
    typename C::value_type def,
    Proj const& proj = {}
)
{
    while (n--)
    {
        b = ltrim_ws(b, e);
        out.emplace_back(proj(parse(b, e, def)));
    }
}

std::optional<mesh> load(std::istream & in)
{
    mesh m;

    std::vector<f32> positions, texcoords, normals;
    std::vector<index> indices;

    constexpr size_t linelen{ 128 };
    char linebuf[linelen];
    char const* b{ linebuf };
    char const* e{ b + linelen };
    while (in.getline(linebuf, linelen))
    {
        e = linebuf + in.gcount();
        b = ltrim_ws(linebuf, e);
        if (e - b < 6) // minimal line is 'vt 0 0'
            continue;

        switch (b[0])
        {
            case 'v':
                switch ((++b)[0])
                {
                    case 'n': parse_n(++b, e, 3, normals, 0.0f); break;
                    case 't': parse_n(++b, e, 2, texcoords, 0.0f); break;
                    default: parse_n(b, e, 3, positions, 0.0f);
                }
            break;

            case 'f': parse_n(++b, e, 3, indices, {}, &decrement); break;
        }
    }

    struct hash_vertex
    {
        sz operator()(vertex const& v) const
        {
            sz r{ 0 };

            for (float a : v.pos)
                hash_combine(r, a);

            for (float a : v.uv)
                hash_combine(r, a);

            for (float a : v.normal)
                hash_combine(r, a);

            return r;
        }
    };

    std::unordered_map<vertex, u32, hash_vertex> uniq_vertices;
    for (auto [v, t, n] : indices)
    {
        vertex vert{
            .pos = {
                positions[v*3 + 0],
                positions[v*3 + 1],
                positions[v*3 + 2],
            },
            .uv = {
                texcoords[t*2 + 0],
                texcoords[t*2 + 1],
            },
            .normal = {
                normals[n*3 + 0],
                normals[n*3 + 1],
                normals[n*3 + 2],
            },
        };

        auto [it, inserted] = uniq_vertices.emplace(vert, m.vertices.size());
        if (inserted)
            m.vertices.push_back(vert);

        m.indices.push_back(it->second);
    }

    return m;
}

}

}

std::optional<mesh> mesh::from_file(std::filesystem::path const& path)
try
{
    std::ifstream in{ path };
    if (!in.is_open())
        return std::nullopt;

    in.exceptions(std::ifstream::badbit);

    if (path.extension() == ".obj")
        return obj::load(in);

    return std::nullopt;
}
catch (std::ifstream::failure const& f)
{
    SDL_Log("[ERROR][engine] can't load mesh: %s\n", f.what());
    return std::nullopt;
}

} // namespace orbi
