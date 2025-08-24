#ifndef FED_AST_HANDLE_HPP_ABSXIUH
#define FED_AST_HANDLE_HPP_ABSXIUH

#include <concepts>
#include <memory>

#include <fmt/core.h>
#include <ranges>
#include <variant>

#include "fed/utils/superutil.hpp"
#include "fed/diagnostics/internal-error.hpp"

namespace fed::ast {

template<typename T>
class handle {
    template<typename>
    friend struct observer_handle;
public:
    using value_type = T;
    handle() 
        : m_handle(nullptr) {}
    template<typename... Us>
    handle(Us&&... args)
        : m_handle(std::make_unique<T>(FWD(args)...)) {}
    handle(handle const&) = delete;
    handle(handle&&) noexcept = default;
    template<typename U>
    handle(handle<U>&& derived)
        : m_handle(std::move(derived)) {}
    constexpr handle(poison_t) noexcept
        :m_handle(nullptr) {}
    auto operator=(handle const&) = delete;
    auto operator=(handle&&) noexcept -> handle& = default;
    template<typename U>
    auto operator=(handle<U>&& derived)
        -> handle& {
        *this = std::move(*derived.m_handle);

        return *this;
    }
    auto and_then(auto&& f) const noexcept {
        if (is_poisoned()) 
            return std::remove_cvref_t<
                std::invoke_result_t<decltype(f), T const&>
            >(poison_pill);
        return std::invoke(FWD(f), *this->m_handle);
    }
    auto transform(auto&& f) const noexcept
        -> handle<std::remove_cvref_t<
                std::invoke_result_t<decltype(f), T const&>
            >> {
        if (is_poisoned()) return poison_pill;
        return std::invoke(FWD(f), *m_handle);
    }
    auto transform(auto&& f) && noexcept
        -> handle<std::remove_cvref_t<
                std::invoke_result_t<decltype(f), T&&>
            >> {
        if (is_poisoned()) return poison_pill;
        return std::invoke(FWD(f), std::move(*m_handle));
    }
    auto operator*()
        -> T& { 
        if (is_poisoned()) throw internal_error("accessing a poisoined handle");
        return *m_handle;
    }
    auto operator*() const 
        -> T const& { 
        if (is_poisoned()) throw internal_error("accessing a poisoined handle");
        return *m_handle;
    }
    auto operator->() 
        -> T* { 
        if (is_poisoned()) throw internal_error("accessing a poisoined handle");
        return m_handle.get(); 
    }
    auto operator->() const
        -> T const* { 
        if (is_poisoned()) throw internal_error("accessing a poisoined handle");
        return m_handle.get(); 
    }
    
    auto get()
        -> T* { 
        if (is_poisoned()) throw internal_error("accessing a poisoined handle");
        return m_handle.get(); 
    }
    auto get() const
        -> T const* { 
        if (is_poisoned()) throw internal_error("accessing a poisoined handle");
        return m_handle.get(); 
    }

    auto poison()
        -> void { m_handle = nullptr; }
    auto is_poisoned() const
        -> bool { return m_handle == nullptr; }
    auto value_or(T val) const
        -> T {
        // requires std::copyable<T> {
        return is_poisoned() ? std::move(val) : *m_handle;
    }
    template<typename Self>
    auto unsafe_value(this Self&& self) noexcept
        -> decltype(auto) { 
        return std::forward_like<Self>(*self.m_handle); 
    }
    auto unsafe_ptr() const noexcept
        -> T* { return m_handle.get(); }

    constexpr auto operator==(handle const& other) const noexcept
        -> bool {
        if (is_poisoned() and other.is_poisoned()) return true;
        if (is_poisoned() or other.is_poisoned()) return false;
        return *m_handle == *other.m_handle;
    }
private:
    std::unique_ptr<T> m_handle{};

};
template<typename U>
handle(U&&) -> handle<std::remove_cvref_t<U>>;
// non - owning, immutable handle
template<typename T>
class observer_handle {
    friend handle<T>;
public:
    using value_type = T;
    observer_handle() = default;
    constexpr observer_handle(poison_t) noexcept 
        : m_handle(nullptr) {}
    
    constexpr observer_handle(T const* ptr)
        : m_handle(ptr) {
    }
    constexpr observer_handle(T const& val)
        : m_handle(std::addressof(val)) {}
    observer_handle(nullptr_t) = delete;
    constexpr observer_handle(handle<T> const& handle)
        : m_handle(handle.m_handle.get()) {}
    constexpr observer_handle(observer_handle const&) noexcept = default;
    constexpr auto operator=(observer_handle const&) noexcept
        -> observer_handle& = default;
    constexpr observer_handle(observer_handle&&) noexcept = default;
    constexpr auto operator=(observer_handle&&) noexcept
        -> observer_handle& = default;
    auto is_poisoned() const noexcept
        -> bool { return m_handle == nullptr; }
    auto and_then(auto&& f) const noexcept {
        if (is_poisoned()) 
            return std::remove_cvref_t<std::invoke_result_t<decltype(f), T const&>>(poison_pill);
        return std::invoke(FWD(f), *this->m_handle);
    }
    template<typename F>
    auto transform(F&& f) const noexcept
        -> observer_handle<std::remove_cvref_t<std::invoke_result_t<F, T const&>>> {
        if (is_poisoned()) { return poison_pill; }
        return std::invoke(FWD(f), *m_handle);
    }

    template<typename F, typename... Ts>
    friend auto then_all(F&& func, observer_handle<Ts>... handles)
        -> std::invoke_result_t<F, Ts const&...>;

    auto value_or(T val) const
        -> T
        requires std::movable<T> {
        return is_poisoned() ? std::move(val) : *m_handle;
    }
    auto ref_or(T&&) = delete;
    auto ref_or(T const& val) const 
        -> T const& { 
        return is_poisoned() ? val : *m_handle;
    }
    auto unsafe_value() const noexcept 
        -> T const& { return *m_handle; }
    auto unsafe_ptr() const noexcept
        -> T const* { return m_handle; }

    constexpr auto operator==(observer_handle const& other) const noexcept
        -> bool {
        if (is_poisoned() and other.is_poisoned()) return true;
        if (is_poisoned() or other.is_poisoned()) return false;
        return *m_handle == *other.m_handle;
    }


private:
    T const* m_handle;
};
template<auto M>
inline constexpr auto try_member = 
    [](observer_handle<member_base<decltype(M)>> hndl) {
        static_assert(member_pointer<decltype(M)>, "expects member pointer");
        using ret_t = decltype(observer_handle(hndl.unsafe_value().*M));
        if (not hndl.is_poisoned()) {
            return observer_handle(hndl.unsafe_value().*M);
        }
        return ret_t(poison_pill);
    };
template<typename T>
inline constexpr auto is = 
    []<typename... Ts>(observer_handle<std::variant<Ts...>> handle) {
        if (handle.is_poisoned()) return true;
        return std::holds_alternative<T>(handle.unsafe_value());
    };
template<typename T>
inline constexpr auto get_if = 
    []<typename... Ts>(observer_handle<std::variant<Ts...>> v) 
    -> observer_handle<T> {
        return std::get_if<T>(v.unsafe_ptr());
    };
inline constexpr auto all_unpoisoned_map =
    [](auto&& r, auto&& out, auto&& map) {
        for (auto&& elem : r) {
            if (not elem.is_poisoned()) {
                *out++ = map(elem.unsafe_value());
            }
        }
    };

template<typename F, typename... Ts>
auto then_all(F&& func, observer_handle<Ts>... handles)
    -> std::invoke_result_t<F, Ts const&...> {
    if ((handles.is_poisoned() or ...)) 
        return std::invoke_result_t<F, Ts const&...>(poison_pill);
    return std::invoke(FWD(func), *handles.m_handle...);
}


} // namespace fed::ast

template<typename T>
struct fmt::formatter<fed::ast::handle<T>> : indentable {
    using this_t = fmt::formatter<fed::ast::handle<T>>;
    constexpr auto parse(fmt::format_parse_context& ctx) {
        return ctx.begin();
    }
    constexpr auto format(fed::ast::handle<T> const& hndl, fmt::format_context& ctx) const {
        if (hndl.is_poisoned()) {
            ctx.out() = indent(ctx);
            return fmt::format_to(ctx.out(), "poisoned");
        } else {
            return fmt::formatter<T>{same_level()}.format(*hndl, ctx);
        }

    }
};

#endif // !FED_AST_HANDLE_HPP_ABSXIUH
