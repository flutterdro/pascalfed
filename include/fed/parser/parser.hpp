#ifndef FED_PARSER_HPP_
#define FED_PARSER_HPP_

#include "fed/representations/raw-source.hpp"
#include "fed/scanner/lex.hpp"
#include "fed/representations/parse-tree.hpp"
#include "fed/parser/parse_error.hpp"
#include "fed/scanner/token.hpp"
#include "fed/utils/superutil.hpp"

#include <expected>
#include <functional>
#include <initializer_list>
#include <type_traits>
#include <algorithm>
#include <ranges>

namespace stdr = std::ranges;

namespace fed {

template<typename T>
using parse_result = std::expected<T, parse_error>;

class parser {
public: 
    explicit parser(std::string_view source);

    auto consume_and_advance()
        -> token_view;
    auto try_consume_and_advance_expecting(token_type token)
        -> std::optional<parse_error>;
    auto current_token()
        -> token_view;
    auto cursor()
        -> source::iterator;

    // every parse function has a contract
    // they must advance lexer to the next token 
    // after parsed source

    auto parse_program() 
        -> parse_result<program>; 
    auto parse_program_heading()
        -> parse_result<program_heading>;
    auto parse_block()
        -> parse_result<block>;
    auto parse_type_definition()
        -> parse_result<type_definition>;
    auto parse_identifier()
        -> parse_result<identifier>;

private:
    lexer m_lexer;
};





} // namespace fed



#endif
