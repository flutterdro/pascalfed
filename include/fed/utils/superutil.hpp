#ifndef FED_SUPERUTIL_HPP_
#define FED_SUPERUTIL_HPP_

#include <ranges>
#include <algorithm>
#include <utility>

namespace fed {

namespace func {

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

} // namespace func


} // namespace fed


#endif
