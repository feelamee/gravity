#pragma once

#include <algorithm>

namespace gt::rng::detail
{

using namespace std::ranges;

template<
    std::indirectly_readable I,
    std::indirectly_regular_unary_invocable<I> Proj
>
using projected_value_t = std::remove_cvref_t<
    std::invoke_result_t<Proj&, std::iter_value_t<I>&>
>;

struct contains_t
{
    template<
        std::input_iterator I,
        std::sentinel_for<I> S,
        class Proj = std::identity,
        class T = projected_value_t<I, Proj>
    >
        requires std::indirect_binary_predicate<
            std::ranges::equal_to,
            std::projected<I, Proj>,
            const T*
        >
    constexpr bool operator()(I first, S last, T const& value, Proj proj = {}) const
    {
        return find(std::move(first), last, value, proj) != last;
    }

    template<
        std::ranges::input_range R,
        class Proj = std::identity,
        class T = projected_value_t<iterator_t<R>, Proj>
    >
        requires std::indirect_binary_predicate<
            equal_to,
            std::projected<iterator_t<R>, Proj>,
            const T*
        >
    constexpr bool operator()(R&& r, T const& value, Proj proj = {}) const
    {
        return find(std::move(begin(r)), end(r), value, proj) != end(r);
    }

    template<
        typename RT,
        class Proj = std::identity,
        class T = projected_value_t<iterator_t<std::initializer_list<RT>>, Proj>
    >
        requires std::indirect_binary_predicate<
            equal_to,
            std::projected<iterator_t<std::initializer_list<RT>>, Proj>,
            const T*
        >
    constexpr bool operator()(std::initializer_list<RT> const& r, T const& value, Proj proj = {}) const
    {
        return find(std::move(begin(r)), end(r), value, proj) != end(r);
    }
};

}

namespace gt::rng
{

using namespace std::ranges;

inline constexpr detail::contains_t contains;

}
