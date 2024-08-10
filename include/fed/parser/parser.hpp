#ifndef FED_PARSER_HPP_
#define FED_PARSER_HPP_

#include "fed/scanner/lex.hpp"
#include "fed/representations/parse-tree.hpp"
#include "fed/parser/parse_error.hpp"
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
    auto current_token()
        -> token_view;

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

    using terminal_list = std::initializer_list<token_type>;
    template<typename ParseFunc>
    using symbol_t = typename std::invoke_result_t<ParseFunc, parser>::value_type;
    template<typename BaseParseFunc>
    auto parse_group(
        BaseParseFunc&& base_parser,
        terminal_list terminals,
        token_type seperator
    ) -> parse_result<group<symbol_t<BaseParseFunc>>> {
        using symbol_actual_t = symbol_t<BaseParseFunc>;
        auto result = group<symbol_actual_t>{};

        {
            auto first = symbol_actual_t{};
            TRY(first, std::invoke(base_parser, *this));
            result.push_back(first);
        }
        while (stdr::none_of(terminals, func::equal_to(current_token().type()))) {
            if (consume_and_advance().type() != seperator) {
                return std::unexpected(parse_error{});
            }
            auto subsequent = symbol_actual_t{};
            TRY(subsequent, std::invoke(base_parser, *this));
            result.push_back(subsequent);
        }

        return result;
    }
private:
    lexer m_lexer;
};





} // namespace fed



#endif
