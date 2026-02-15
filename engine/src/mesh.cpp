#include <engine/mesh.hpp>

#include <fstream>
#include <algorithm>
#include <charconv>
#include <strings.h>
#include <unordered_map>
#include <cassert>
#include <filesystem>
#include <numeric>

#include <SDL3/SDL_log.h>
#include <SDL3/SDL_assert.h>

namespace gt::obj
{

static bool isws(char c)
{
    return c <= ' ';
}

static char const* ltrim_ws(char const* b, char const* e)
{
    return std::find_if_not(b, e, &isws);
}

static char const* ltrim_n(
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
static T parse(char const*& b, char const* e, T v)
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

static index decrement(index v)
{
    v.v -= 1;
    v.t -= 1;
    v.n -= 1;

    return v;
}

template<typename C, typename Proj = std::identity>
static void parse_n(
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

static bool load(mesh & m, std::istream & in)
{
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

    if (!positions.empty())
        m.attribs.push_back(attrib::pos);
    if (!texcoords.empty())
        m.attribs.push_back(attrib::uv);
    if (!normals.empty())
        m.attribs.push_back(attrib::normal);

    struct vertex
    {
        f32 pos[attrib_size(attrib::pos)];
        f32 uv[attrib_size(attrib::uv)];
        f32 normal[attrib_size(attrib::normal)];

        constexpr auto operator<=>(vertex const&) const = default;
    };

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
        vertex vert{};

        if (!positions.empty())
        {
            sz size = attrib_size(attrib::pos);
            for (sz i = 0; i < size; ++i)
                vert.pos[i] = positions[v*size + i];
        }

        if (!texcoords.empty())
        {
            sz size = attrib_size(attrib::uv);
            for (sz i = 0; i < size; ++i)
                vert.uv[i] = texcoords[t*size + i];
        }

        if (!normals.empty())
        {
            sz size = attrib_size(attrib::normal);
            for (sz i = 0; i < size; ++i)
                vert.normal[i] = normals[n*size + i];
        }

        SDL_assert(m.vertices.size() % m.stride() == 0);
        auto [it, inserted] = uniq_vertices.emplace(vert, m.vertices.size() / m.stride());
        if (inserted)
        {
            if (!positions.empty())
                m.vertices.insert(end(m.vertices), vert.pos, vert.pos + attrib_size(attrib::pos));

            if (!texcoords.empty())
                m.vertices.insert(end(m.vertices), vert.uv, vert.uv + attrib_size(attrib::uv));

            if (!normals.empty())
                m.vertices.insert(end(m.vertices), vert.normal, vert.normal + attrib_size(attrib::normal));
        }

        m.indices.push_back(it->second);
    }

    return true;
}

}

namespace gt
{

sz mesh::stride() const
{
    sz r = 0;
    for (attrib a : attribs)
        r += attrib_size(a);

    return r;
}

sz mesh::bytestride() const
{
    sz r = 0;
    for (attrib a : attribs)
        r += attrib_bytesize(a);

    return r;
}

bool from_file(mesh & m, std::filesystem::path const& path)
try
{
    std::ifstream in{ path };
    if (!in.is_open())
        return false;

    in.exceptions(std::ifstream::badbit);

    auto const ext = path.extension();
    if (ext == ".obj")
        return obj::load(m, in);

    SDL_Log("[ERROR][engine] can't load mesh: unsupported format %s\n", ext.c_str());
    return false;
}
catch (std::ifstream::failure const& f)
{
    SDL_Log("[ERROR][engine] can't load mesh: %s\n", f.what());
    return false;
}

} // namespace gt
