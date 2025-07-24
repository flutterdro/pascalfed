#ifndef FED_PARSE_ERROR_HPP_
#define FED_PARSE_ERROR_HPP_

#include "fed/diagnostics/compile-error.hpp"
#include "fed/representations/raw-source.hpp"
#include "fed/scanner/token.hpp"

#include <string>
#include <optional>

namespace fed {

template<typename T>
class recovery_result : std::optional<T> {
    using base_t = std::optional<T>;
public:
    using base_t::optional;
    using base_t::operator=;
    using base_t::value_or;
    using base_t::transform;
    using base_t::and_then;
    using base_t::or_else;
    using base_t::has_value;
    using base_t::operator*;
    using base_t::operator->;
    using base_t::operator bool;
    template<typename U, typename V>
    using copy_cref_t = decltype(std::forward_like<U>(std::declval<V>()));
    template<typename Self>
    constexpr auto cast_to_base(this Self&& self) 
        -> copy_cref_t<Self, base_t> {

    }
    template<typename Self>
    constexpr explicit operator base_t (this Self&& self)  {
        return std::forward_like<Self>(self);
    }
};

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
