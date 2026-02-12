#pragma once

#include <ranges>

namespace gt::rng::detail
{

using namespace std::ranges;

template<typename Size, range R>
constexpr auto impl(R&& r)
{
    return transform_view(
        std::forward<R>(r),
        [i = Size(0)](auto&& v) mutable
        {
            return std::pair{ i++, std::forward<decltype(v)>(v) };
        }
    );
}

template<typename Size>
struct enumerate_with_t
{
    template<range R>
    constexpr auto operator()(R&& r) const
    {
        return impl<Size>(std::forward<R>(r));
    }

    template<typename T>
    constexpr auto operator()(std::initializer_list<T> const& r) const
    {
        return impl<Size>(r);
    }
};

struct enumerate_t
{
    template<range R>
    constexpr auto operator()(R&& r) const
    {
        return impl<range_size_t<R>>(std::forward<R>(r));
    }

    template<typename T>
    constexpr auto operator()(std::initializer_list<T> const& r) const
    {
        return impl<range_size_t<std::initializer_list<T>>>(r);
    }
};

}

namespace gt::rng
{

using namespace std::ranges;

inline constexpr detail::enumerate_t enumerate_view;

template<typename Size>
inline constexpr detail::enumerate_with_t<Size> enumerate_with_view;

}

namespace gt::vs
{

using namespace std::views;

inline constexpr rng::detail::enumerate_t enumerate;

template<typename Size>
inline constexpr rng::detail::enumerate_with_t<Size> enumerate_with;

}
