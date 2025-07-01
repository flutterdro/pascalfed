#ifndef FED_PREDICATES_HPP_
#define FED_PREDICATES_HPP_

#include "fed/utils/macros.hpp"
#include <algorithm>
#include <functional>
#include <ranges>
namespace stdr = std::ranges;
namespace fed {
template<typename T>
concept predicate = requires { typename std::remove_cvref_t<T>::predicate_t; };
template<typename Pred>
struct predicate_wrap : private Pred {
    using predicate_t = void;
    constexpr predicate_wrap(Pred&& pred) 
        : Pred(std::move(pred)) {}
    using Pred::operator();
};
template<typename Pred>
struct enable_partially_applied_predicate {
    constexpr auto operator()(auto&& val) const {
        return predicate_wrap(std::bind_back(Pred(), FWD(val)));
    }
};
inline constexpr struct equal_to_t 
    : enable_partially_applied_predicate<equal_to_t> {
    using predicate_t = void;
    using enable_partially_applied_predicate::operator();
    constexpr auto operator()(auto const& lhs, auto const& rhs) const
        -> bool { return lhs == rhs; }
} equal_to;
inline constexpr struct any_of_t 
    : enable_partially_applied_predicate<any_of_t> {
    using predicate_t = void;
    using enable_partially_applied_predicate::operator();
    constexpr auto operator()(auto const& val, stdr::range auto&& range) const
        -> bool { return stdr::any_of(range, equal_to(val)); }
} any_of;
// technicall could be implemented as 
// inline constexpr auto none_of = not any_of;
// lmao
inline constexpr struct none_of_t 
    : enable_partially_applied_predicate<none_of_t> {
    using predicate = void;
    using enable_partially_applied_predicate::operator();
    constexpr auto operator()(auto const& val, stdr::range auto&& range) const
        -> bool { return stdr::none_of(range, equal_to(val)); }
} none_of;
constexpr auto operator and(predicate auto&& pred1, predicate auto&& pred2) {
    return predicate_wrap([pred1_ = FWD(pred1), pred2_ = FWD(pred2)](auto const& x) {
        return std::invoke(pred1_, x) and std::invoke(pred2_, x);
    });
};
constexpr auto operator or(predicate auto&& pred1, predicate auto&& pred2) {
    return predicate_wrap([pred1_ = FWD(pred1), pred2_ = FWD(pred2)](auto const& x) {
        return std::invoke(pred1_, x) or std::invoke(pred2_, x);
    });
};
constexpr auto operator not(predicate auto&& pred1) {
    return predicate_wrap([pred1_ = FWD(pred1)](auto const& x) {
        return not std::invoke(pred1_, x);
    });
};

} // namespace fed


#endif
