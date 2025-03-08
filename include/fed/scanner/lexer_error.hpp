#ifndef FED_LEXER_ERROR_HPP
#define FED_LEXER_ERROR_HPP

#include <string>

#include "fed/representations/raw-source.hpp"

namespace fed {
class lexer_error {
    enum class type {
        invalid_escape,
        untermintated_literal,
        invalid_literal,
    };
public:
    auto message() const
        -> std::string { return {}; }
    auto location() const
        -> source::location { return {}; }
private:
};
} // namespace fed


#endif
