#ifndef FED_AST_HANDLE_HPP_ABSXIUH
#define FED_AST_HANDLE_HPP_ABSXIUH

#include <memory>

#include <fmt/core.h>

#include "fed/utils/superutil.hpp"
#include "fed/diagnostics/internal-error.hpp"

namespace fed::ast {

template<typename T>
class handle {
    template<typename>
    friend struct observer_handle;
public:
    handle() 
        : m_handle(nullptr) {}
    handle(T&& val)
        : m_handle(std::make_unique<T>(std::move(val))) {}
    handle(handle const&) = delete;
    handle(handle&&) noexcept = default;
    template<typename U>
    handle(handle<U>&& derived)
        : m_handle(std::move(derived)) {}
    auto operator=(T&& val)
        -> handle& {
        m_handle = std::make_unique<T>(std::move(val));

        return *this;
    }
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
private:
    std::unique_ptr<T> m_handle{};

};
// non - owning, immutable handle
template<typename T>
class observer_handle {
    friend handle<T>;
public:
    observer_handle() = delete;
    constexpr observer_handle(poison_t) noexcept 
        : m_handle(nullptr) {}
    
    constexpr observer_handle(T const* ptr)
        : m_handle(ptr) {
        if (ptr == nullptr) throw internal_error("observer_handle cannot be constructed from nullptr");
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

    template<typename F, typename... Ts>
    friend auto then_all(F&& func, observer_handle<Ts>... handles)
        -> std::invoke_result_t<F, Ts const&...>;

    auto unsafe_value() const noexcept 
        -> T const& { return *m_handle; }

private:
    T const* m_handle;
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
            return fmt::format_to(ctx.out(), "poisoned\n");
        } else {
            return fmt::formatter<T>{same_level()}.format(*hndl, ctx);
        }

    }
};

#endif // !FED_AST_HANDLE_HPP_ABSXIUH
