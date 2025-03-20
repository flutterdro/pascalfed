#ifndef FED_SUPERUTIL_HPP_
#define FED_SUPERUTIL_HPP_

#include <ranges>
#include <algorithm>
#include <utility>

#include <fmt/core.h>

#include "fed/utils/macros.hpp"

class indentable {
public:
    constexpr indentable() 
        : level(0) {}
    constexpr auto new_level() const noexcept {
        auto new_level = indentable();
        new_level.level = level + 1;
        return new_level;
    }
    constexpr auto same_level() const noexcept {
        auto new_level = indentable();
        new_level.level = level;
        return new_level;
    }

    constexpr auto indent(fmt::format_context& ctx) const {
        for (unsigned i = 0; i < level; ++i) {
            ctx.out()++ = ' ';
            ctx.out()++ = ' ';
        }
        ctx.out()++ = '|';
        return ctx.out();
    }
private:
    unsigned level;

};

namespace fed {

struct non_copyable { 
    constexpr non_copyable(non_copyable const&) = delete;
    constexpr auto operator=(non_copyable const&)
        -> non_copyable& = delete;
};

struct non_movable {
    constexpr non_movable(non_movable&&) = delete;
    constexpr auto operator=(non_movable&&)
        -> non_movable& = delete;
};

template<typename... Ts>
struct overloaded : Ts... {
    using Ts::operator()...;
};

namespace func {

inline constexpr auto dereference = [](auto&& pointer) 
    -> decltype(auto) { 
    return *pointer;
};

inline constexpr auto deep_dereference = [](auto&& pointer) 
    -> decltype(auto) { 
    return std::forward_like<decltype(pointer)>(*pointer);
};

namespace detail {
template<typename F>
struct func_wrap : F {
    using F::operator();
    constexpr friend auto operator|(auto&& f1, auto&& f2) {
        return [_f1 = FWD(f1), _f2 = FWD(f2)](auto&&... xs) {
            return _f1(_f2(FWD(xs)...));
        };
    }
};
}

inline constexpr auto be_combinator = [](auto&&... funcs) {
    return [..._funcs = detail::func_wrap(FWD(funcs))](auto&& x) {
        return (_funcs | ...)(FWD(x));
    };
};

inline constexpr auto b1e_combinator = [](auto&& f, auto&& g) {
    return [_f = FWD(f), _g = FWD(g)](auto&&... xs) {
        return _f(_g(FWD(xs)...));
    };
};

inline constexpr auto psie_combinator = [](auto&& f, auto&& g) {
    return [_f = FWD(f), _g = FWD(g)](auto&&... xs) {
        return _f(_g(FWD(xs))...);
    };
};

namespace stdr = std::ranges;
struct static_any {
    template<typename T>
    constexpr operator T() {}
};

} // namespace func


} // namespace fed


#endif
