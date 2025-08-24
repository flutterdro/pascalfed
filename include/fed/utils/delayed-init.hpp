#include "fed/utils/macros.hpp"
#include <utility>
#include <type_traits>
#include <functional>

namespace fed {
inline constexpr struct invoke_in_place_t {} invoke_in_place{}; 
template<typename T>
class delayed_init {
public:
    constexpr delayed_init() noexcept
        : m_dummy('\x42') {}

    constexpr delayed_init(delayed_init const& other)
        requires std::is_trivially_copy_constructible_v<T>
        = default;
    constexpr delayed_init(delayed_init&& other) noexcept 
        requires std::is_trivially_move_constructible_v<T>
        = default;
    constexpr auto operator=(delayed_init const& other) 
        -> delayed_init& 
        requires std::is_trivially_copy_assignable_v<T>
        = default;
    constexpr auto operator=(delayed_init const& other) 
        -> delayed_init&
        requires std::is_trivially_move_assignable_v<T>
        = default;
    constexpr ~delayed_init() 
        requires std::is_trivially_destructible_v<T> 
        = default;

    constexpr delayed_init(delayed_init const& other)
        : m_value(other.m_value) {}
    constexpr delayed_init(delayed_init&& other) noexcept
        : m_value(std::move(other.m_value)) {}
    constexpr auto operator=(delayed_init const& other) 
        -> delayed_init& {
        m_value = other.m_value;
        return *this;
    }
    constexpr auto operator=(delayed_init&& other) 
        -> delayed_init& {
        m_value = std::move(other.m_value);
        return *this;
    }
    constexpr ~delayed_init()
        requires (not std::is_trivially_destructible_v<T>) {
        m_value.~T();
    }

    template<typename U = T>
    constexpr delayed_init(U&& value)
        : m_value(FWD(value)) {}
    template<typename... Ts>
    constexpr delayed_init(std::in_place_t, Ts&&... args)
        : m_value(FWD(args)...) {}
    template<typename F, typename... Ts>
    constexpr delayed_init(invoke_in_place_t, F&& f, Ts&&... args)
        : m_value(std::invoke(FWD(f), FWD(args)...)) {}

    template<typename Self>
    constexpr auto get(this Self&& self) noexcept
        -> decltype(auto) {
        return std::forward_like<Self>(self.m_value);
    }
private:
    union {
        char m_dummy;
        T    m_value;
    };
};
}
