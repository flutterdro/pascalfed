#ifndef FED_PARSE_ERROR_HPP_
#define FED_PARSE_ERROR_HPP_

#include "fed/diagnostics/compile-error.hpp"
#include "fed/parser/context.hpp"
#include "fed/representations/raw-source.hpp"
#include "fed/scanner/token.hpp"
#include "fed/utils/superutil.hpp"

#include <string>
#include <optional>
#include <expected>
#include <type_traits>
#include <variant>

namespace fed {
template<typename>
class identity_monad;
template<typename>
struct check : std::false_type {};
template<typename U>
struct check<identity_monad<U>> : std::true_type {
};
template<typename U>
concept is_identity_monad = check<U>::value;

template<typename T>
class identity_monad {

public:
    using value_type = T;
    using error_type = std::monostate;

    constexpr identity_monad() = default;
    constexpr identity_monad(identity_monad&&) noexcept = default;
    constexpr identity_monad(identity_monad const&) = default;

    constexpr auto operator=(identity_monad&&) noexcept
        -> identity_monad& = default;
    constexpr auto operator=(identity_monad const&)
        -> identity_monad& = default;

    constexpr identity_monad(uref<value_type> auto&& val_)
        : val(FWD(val_)) {}
    constexpr identity_monad(error_type const&) = delete;

    template<typename Self>
    constexpr auto operator*(this Self&& self) noexcept
        -> decltype(auto) { return std::forward_like<Self>(self.val); }
    static consteval auto has_value() noexcept
        -> bool { return true; }
    static consteval auto error() noexcept
        -> error_type { return {}; }
    template<typename Self>
    constexpr auto value_or(this Self&& self, auto&&) noexcept
        -> decltype(auto) { return std::forward_like<Self>(self.val); }
    template<typename Self>
    constexpr auto transform(this Self&& self, std::invocable<T> auto&& func) 
        -> decltype(auto) { 
        using ret_t = std::invoke_result_t<decltype(func), T>;
        return identity_monad<ret_t>(std::invoke(
            FWD(func), std::forward_like<Self>(self.val)
        )); 
    }
    template<typename Self>
    constexpr auto and_then(this Self&& self, auto&& func) 
        -> decltype(auto) { 
        static_assert(
            is_identity_monad<std::invoke_result_t<decltype(func), Self>>,
            "result of monadic bind must be the same monad"
        );
        return std::invoke(
            FWD(func), std::forward_like<Self>(self.val)
        ); 
    }
    template<typename Self>
    constexpr auto or_else(this Self&& self, auto&&)
        -> decltype(auto) {
        return FWD(self);
    }
private:
    T val;
};

template<typename T>
class optional_mono : std::optional<T> {
    using base_t = std::optional<T>;
public:
    using typename base_t::value_type;
    using error_type = std::monostate;
    constexpr optional_mono(std::monostate) noexcept
        : base_t(std::nullopt) {}
    using base_t::optional;
    using base_t::operator=;
    using base_t::operator bool;
    using base_t::operator->;
    using base_t::operator*;
    using base_t::has_value;
    using base_t::emplace;
    using base_t::swap;
    using base_t::reset;
    using base_t::value_or;
    constexpr auto error() const noexcept
        -> error_type { return {}; }
    template<typename Self, typename F>
    constexpr auto transform(this Self&& self, F&& func) {
        using res_t = std::invoke_result_t<F, copy_cref<Self, T>>;
        if (self.has_value()) {
            return optional_mono<res_t>(
                std::invoke(FWD(func), *FWD(self))
            );
        }
        return optional_mono<res_t>(error_type());
    }
    template<typename Self, typename F>
    constexpr auto and_then(this Self&& self, F&& func) {
        using res_t = std::invoke_result_t<F, copy_cref<Self, T>>;
        if (self.has_value()) {
            return optional_mono<res_t>(
                std::invoke(FWD(func), *FWD(self))
            );
        }
        return res_t(error_type());
    }
};
template<typename T>
identity_monad(T&& val_) -> identity_monad<T>;
template<typename T>
using parse_result = std::expected<T, compilation_error>;
template<typename T> 
using recovery_result = optional_mono<T>;
inline constexpr auto recovery_fail = std::monostate();
template<typename T>
using many_parse_result = identity_monad<T>;

class parser;
class semantic_context;
template<typename ParseF>
using get_parse_result = std::invoke_result_t<ParseF, parser&, semantic_context&>;
template<typename ParseF>
using get_result_node = get_parse_result<ParseF>::value_type;
namespace detail {
template<typename, template<typename...> typename>
struct same_template_impl : std::false_type {};
template<typename... Ts, template<typename...> typename Base1>
struct same_template_impl<Base1<Ts...>, Base1> : std::true_type {
};
}
template<typename T, template<typename...> typename Base>
concept same_template = detail::same_template_impl<T, Base>::value; 
template<typename ParseF>
concept monitored = (
    same_template<get_parse_result<ParseF>, identity_monad> or 
    same_template<get_parse_result<ParseF>, optional_mono>);

class missing_token {
public:
    constexpr missing_token(
        source::location loc, 
        token_type token
    ) : m_location(loc), m_token(token) {}
    constexpr auto location() const noexcept
        -> source::location { return m_location; }
    constexpr auto message() const noexcept
        -> std::string {
        return "Missing token: '" + to_string(m_token) + "'";
    }
private:
    source::location m_location;
    token_type       m_token;
};

class parse_error  {
public:
    enum class type {
        missing_token,
        unexpected_token,
    };
    parse_error() = default;

    parse_error(
        source::location location,
        source::view region,
        type type, token_type expected_token
    )
        : parse_error() {}
        
    auto message() const 
        -> std::string { return ""; }
    auto location() const
        -> source::location { return m_location; }
    
    auto set_region(source::view region) noexcept
        -> void;
 
    ~parse_error() = default;
private:
    source::location m_location;
    source::view m_region;
    type m_type;
    token_type m_expected_token;
};

}

#endif // !FED_PARSE_ERROR_HPP_
