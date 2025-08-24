#ifndef FED_SEMANTIC_ERRORS_HPP_
#define FED_SEMANTIC_ERRORS_HPP_

#include "fed/diagnostics/buffer.hpp"
#include "fed/diagnostics/compile-error.hpp"
#include "fed/representations/ast/forward.hpp"
#include "fed/representations/ast/name-scope.hpp"
#include "fed/representations/raw-source.hpp"
#include "fed/utils/delayed-init.hpp"

#include <fmt/core.h>
#include <format>

namespace fed {

template<typename T>
class semantic_result {
public:
    template<typename U>
    friend class semantic_result;
    constexpr semantic_result() noexcept = default;
    constexpr semantic_result(semantic_result const&) = default;
    constexpr semantic_result(semantic_result&&) noexcept = default;
    constexpr auto operator=(semantic_result const&) 
        -> semantic_result& = default;
    constexpr auto operator=(semantic_result&&) noexcept
        -> semantic_result& = default;

    template<typename U = T>
    constexpr auto set_result(U&& res) 
        -> void { m_result = FWD(res); }
    constexpr auto add_error(compilation_error error)
        -> void { m_errors.push_back(std::move(error)); }
    template<typename U>
    constexpr auto sieve(semantic_result<U>&& other)
        -> U { 
        for (auto&& error : other.m_errors) {
            m_errors.push_back(std::move(error));
        }
        if constexpr (std::is_same_v<U, void>) {
            return;
        } else {
            return std::move(other.m_result).get();
        }
    }
    constexpr auto sploink(diagnostics_buffer& diag) &&
        -> T {
        for (auto&& error : m_errors) {
            diag.push_back(std::move(error));
        }
        return std::move(m_result).get();
    }
private:
    delayed_init<T> m_result;
    std::vector<compilation_error> m_errors;
};
template<>
class semantic_result<void> {
public:
    template<typename U>
    friend class semantic_result;
    constexpr semantic_result() noexcept = default;
    constexpr semantic_result(semantic_result const&) = default;
    constexpr semantic_result(semantic_result&&) noexcept = default;
    constexpr auto operator=(semantic_result const&) 
        -> semantic_result& = default;
    constexpr auto operator=(semantic_result&&) noexcept
        -> semantic_result& = default;

    constexpr auto add_error(compilation_error error)
        -> void { m_errors.push_back(std::move(error)); }
    template<typename U>
    constexpr auto sieve(semantic_result<U>&& other)
        -> U { 
        for (auto&& error : other.m_errors) {
            m_errors.push_back(std::move(error));
        }
        if constexpr (std::is_same_v<U, void>) {
            return;
        } else {
            return std::move(other.m_result).get();
        }
    }
    constexpr auto sploink(diagnostics_buffer& diag) &&
        -> void {
        for (auto&& error : m_errors) {
            diag.push_back(std::move(error));
        }
    }
private:
    std::vector<compilation_error> m_errors;
};
class insertion_error {
public:
    auto message() const 
        -> std::string {
        auto to_string = [](ast::symbol_kind kind) {
            switch (kind) {
                using enum ast::symbol_kind;
                using namespace std::literals;
            case type:      return "type"s;
            case variable:  return "variable"s;
            case constant:  return "constant"s;
            case function:  return "function"s;
            case procedure: return "procedure"s;
              break;
            }
        };
        return fmt::format(
            "name '{}' is already used in this scope to define a {}",
            m_name, to_string(m_preventee)
        );
    }
    auto location() const
        -> source::location { return {}; }
private:
    ast::symbol_kind m_preventee;
    ast::identifier  m_name;

};
class contextual_error {
    enum class code {
        missing_definition,
        identifier_ocupied,
    };
public:
    contextual_error() = default;
    contextual_error(code code, source::view identifier)
        : m_code(code), m_identifier(identifier) {}
    auto message() const 
        -> std::string {
        switch (m_code) {
            case code::missing_definition: {
                return std::format(
                    "identifier '{}' is used but not defined", 
                    m_identifier.base()
                );
            }
            case code::identifier_ocupied: {
                return std::format(
                    "identifier '{}' is already defined", 
                    m_identifier.base()
                );
            }
        }
    }
    auto location() const
        -> source::location { return {}; }
    code m_code;
    source::view m_identifier;

};

class typecheck_error {

};


} // namespace fed


#endif

