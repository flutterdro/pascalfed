#ifndef FED_MACROS_HPP_
#define FED_MACROS_HPP_


#define FWD(expr) std::forward<decltype(expr)>(expr)
#define LIFT(func) [](auto&&... xs) { return func(FWD(xs)...); }
#define LIFT_MEMBER(func) [this](auto&&... xs) { return this->func(FWD(xs)...); }

#endif
