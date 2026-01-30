#pragma once

#include <engine/fundamental.hpp>

#include <type_traits>
#include <string_view>
#include <source_location>

namespace gt
{

template<typename E>
    requires std::is_enum_v<E>
auto to_underlying(E e)
{
    using t = std::underlying_type_t<E>;
    return static_cast<t>(e);
}

struct noncopyable
{
    noncopyable() = default;

    noncopyable(noncopyable const&) = delete;
    noncopyable& operator=(noncopyable const&) = delete;

    noncopyable(noncopyable &&) = default;
    noncopyable& operator=(noncopyable &&) = default;
};

[[noreturn]] void unimplemented(
    std::source_location = std::source_location::current()
);

[[noreturn]] inline void
unreachable()
{
    // Uses compiler specific extensions if possible.
    // Even if no extension is used, undefined behavior is still raised by
    // an empty function body and the noreturn attribute.
#if defined(_MSC_VER) && !defined(__clang__) // MSVC
    __assume(false);
#else // GCC, Clang
    __builtin_unreachable();
#endif
}

// TODO! make ctor tempalte instead of whole class (type-erasure)
template<typename Fn>
class scope_exit
{
public:
    scope_exit(Fn fn)
        : fn(std::move(fn))
    {
    }

    ~scope_exit()
    {
        fn();
    }

    scope_exit(scope_exit const&) = delete;
    scope_exit& operator=(scope_exit const&) = delete;

    scope_exit(scope_exit && o) noexcept
        : fn(std::move(o.fn))
    {
    }

    scope_exit& operator=(scope_exit o) noexcept
    {
        swap(*this, o);
        return *this;
    }

    void swap(scope_exit& o) noexcept
    {
        using std::swap;
        swap(fn, o.fn);
    }

private:
    Fn fn;
};

namespace detail
{

void log_debug(int line, char const* fn, char const* fmt, ...);

} // namespace detail

} // namespace gt

// TODO! remove or move to separate header
#define GT_CONCAT(a, b) a ## b
#define GT_UNIQUE_ID(l) GT_CONCAT(UNIQUE_ID_, l)
#define GT_SCOPE_EXIT [[maybe_unused]] ::gt::scope_exit GT_UNIQUE_ID(__LINE__) = [&]

#ifndef NDEBUG
#define GT_LOG_DEBUG(...) ::gt::detail::log_debug(__LINE__, __FILE__, __VA_ARGS__)
#else
#define LOG_DEBUG(...)
#endif

