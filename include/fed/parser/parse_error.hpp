#ifndef FED_PARSE_ERROR_HPP_
#define FED_PARSE_ERROR_HPP_

#include "fed/diagnostics/compile-error.hpp"
#include "fed/representations/raw-source.hpp"
#include "fed/scanner/token.hpp"

#include <string>

namespace fed {

class parse_error : compilation_error {
public:
    enum class type {
        missing_token,
        unexpected_token,
    };

    parse_error(
        source::location location,
        source::view region,
        type type, token_type expected_token
    );
        
    auto message() const 
        -> std::string override;
    auto location() const
        -> source::location override;
    
    auto set_region(source::view region) noexcept
        -> void;
 
    ~parse_error() override = default;
private:
    source::location m_location;
    source::view m_region;
    type m_type;
    token_type m_expected_token;
};

}

#endif // !FED_PARSE_ERROR_HPP_
