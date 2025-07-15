#ifndef FED_SEMANTIC_ERRORS_HPP_
#define FED_SEMANTIC_ERRORS_HPP_

#include "fed/representations/ast/forward.hpp"
#include "fed/representations/ast/name-scope.hpp"
#include "fed/representations/raw-source.hpp"

#include <fmt/core.h>
#include <format>

namespace fed {

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

