#ifndef FED_SUPERUTIL_HPP_
#define FED_SUPERUTIL_HPP_

#include <ranges>
#include <algorithm>
#include <utility>

#define FWD(expr) std::forward<decltype(expr)>(expr)
#define LIFT(func) [](auto&&... xs) { return func(FWD(xs)...); }
#define LIFT_MEMBER(func) [this](auto&&... xs) { return this->func(FWD(xs)...); }


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
constexpr auto equal_to(auto&& value) noexcept {
    return [=](auto&& other) {
        return other == value;
    };
}

template<std::ranges::range R>
constexpr auto any_of(R&& collection) noexcept {
    return [collection_ = std::forward<R>(collection)](auto&& element) {
        return stdr::any_of(collection_, func::equal_to(element));
    };
}
constexpr auto no(auto&& func) {
    return [func_ = std::forward<decltype(func)>(func)](auto&&... args) -> bool {
        return not func_(std::forward<decltype(args)>(args)...);
    };
}
struct static_any {
    template<typename T>
    constexpr operator T() {}
};
constexpr auto operator or(auto&& lhs, auto&& rhs) {
    return [_lhs = FWD(lhs), _rhs = FWD(rhs)](auto&& val) {
        return _lhs(val) or _rhs(val);
    };
}
constexpr auto operator and(auto&& lhs, auto&& rhs) {
    return [_lhs = FWD(lhs), _rhs = FWD(rhs)](auto&& val) {
        return _lhs(val) and _rhs(val);
    };
}

constexpr auto operator not(auto&& rhs) {
    return [_rhs = FWD(rhs)](auto&& val) {
        return not _rhs(val);
    };
}

} // namespace func


} // namespace fed


#endif
