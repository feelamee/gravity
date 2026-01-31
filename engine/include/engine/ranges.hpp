#pragma once

#include <ranges>

namespace gt
{

namespace rng
{

using namespace std::ranges;

auto const enumerate_view = []<range R>(R&& r)
{
    using size_type = range_size_t<R>;
    return transform_view(
        std::forward<R>(r),
        [i = size_type(0)](auto&& v) mutable
        {
            return std::pair{ i++, std::forward<decltype(v)>(v) };
        }
    );
};

}

namespace vs
{
    using namespace std::views;

    constexpr auto enumerate = rng::enumerate_view;
}

}
