#ifndef FED_PREDICATES_HPP_
#define FED_PREDICATES_HPP_

#include "fed/utils/macros.hpp"
#include <algorithm>
#include <ranges>
namespace stdr = std::ranges;
namespace fed {
inline constexpr auto equal_to = [](auto&& val) {
    return [val_ = FWD(val)](auto&& other) {
        return val_ == other;
    };
};
inline constexpr auto any_of = [](stdr::range auto&& range) {
    return [range_ = FWD(range)](auto&& val) {
        return stdr::any_of(range_, equal_to(FWD(val)));
    };
};
inline constexpr auto none_of = [](stdr::range auto&& range) {
    return [range_ = FWD(range)](auto&& val) {
        return stdr::none_of(range_, equal_to(FWD(val)));
    };
};
constexpr auto operator and(auto&& pred1, auto&& pred2) {
    return [pred1_ = FWD(pred1), pred2_ = FWD(pred2)](auto&& x) {
        return std::invoke(pred1_, FWD(x)) and std::invoke(pred2_, FWD(x));
    };
};
constexpr auto operator or(auto&& pred1, auto&& pred2) {
    return [pred1_ = FWD(pred1), pred2_ = FWD(pred2)](auto&& x) {
        return std::invoke(pred1_, FWD(x)) or std::invoke(pred2_, FWD(x));
    };
};
constexpr auto operator not(auto&& pred1) {
    return [pred1_ = FWD(pred1)](auto&& x) {
        return not std::invoke(pred1_, FWD(x));
    };
};

} // namespace fed


#endif
