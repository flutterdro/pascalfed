#ifndef FED_SEMANTIC_ERRORS_HPP_
#define FED_SEMANTIC_ERRORS_HPP_

#include "fed/representations/raw-source.hpp"
#include <format>

namespace fed {

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

