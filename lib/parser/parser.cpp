#include "fed/parser/parser.hpp"
#include "fed/parser/parse_error.hpp"
#include "fed/representations/parse-tree.hpp"
#include "fed/scanner/token.hpp"
#include "fed/utils/superutil.hpp"
#include <algorithm>
#include <initializer_list>
#include <optional>
#include <type_traits>


namespace fed {

// I kind of miss rust's operator ?
#define TRY(dest, exp) \
do {auto exp2 = exp;\
if (not exp2.has_value()) return std::unexpected{std::move(exp2.error())}; \
dest = std::move(exp2.value());} while(false)


using list_of_terminals = std::initializer_list<token_type>;
template<typename SymbolT>
using parse_function_ptr = auto (parser::*)() -> parse_result<SymbolT>;
template<typename SymbolT>
constexpr auto parse_group_of_symbols(
    parser& parser, 
    parse_function_ptr<SymbolT> symbol_parse_function,
    list_of_terminals terminals,
    token_type separator
) -> parse_result<group<SymbolT>> {
    auto result = group<SymbolT>{};

    {
        auto element = SymbolT{};
        TRY(element, (parser.*symbol_parse_function)());
        result.push_back(std::move(element));
    }

    while (stdr::none_of(terminals, func::equal_to(parser.current_token().type()))) {
        if (parser.consume_and_advance().type() != separator) {
            return std::unexpected(parse_error{});
        }
        
        auto element = SymbolT{};
        TRY(element, (parser.*symbol_parse_function)());
        result.push_back(std::move(element));
    }

    
    return result;
}


// program = program-heading ';' block
auto parser::parse_program()
    -> parse_result<program> {
    // std::expected inhibits rvo
    // for now I don't give a fuck about perfomance
    // could be an issue later
    auto result = program{};

    TRY(result.head, parse_program_heading());

    if (consume_and_advance().m_type != token_type::semicolon) {
        return std::unexpected(parse_error{});
    }
    
    TRY(result.body, parse_block());
    
    return result;
}

// program-heading = 'program' identifier ['(' identifier-list ')']
auto parser::parse_program_heading()
    -> parse_result<program_heading> {
    auto result = program_heading{};

    if (consume_and_advance().m_type != token_type::keyword_program) {
        return std::unexpected(parse_error{});
    }

    TRY(result.name, parse_identifier());

    if (m_lexer.lex_next_token().m_type != token_type::l_paren) {
        result.externals = std::nullopt;
        return result;
    }
    m_lexer.advance_lexer();

    TRY(
        result.externals, 
        parse_group_of_symbols(
            *this, 
            &parser::parse_identifier, 
            {token_type::r_paren}, 
            token_type::comma
        )
    );

    auto first_identifier = identifier{};
    TRY(first_identifier, parse_identifier());
    result.externals->push_back(first_identifier);
    while (m_lexer.lex_next_token().m_type != token_type::r_paren) {
        if (consume_and_advance().m_type != token_type::comma) {
            return std::unexpected(parse_error{});
        }
        auto next_identifier = identifier{};
        TRY(next_identifier, parse_identifier());
        result.externals->push_back(next_identifier);
    }
    m_lexer.advance_lexer();
    
    return result;
}

// block = label-declaration-part constant-definition-part
//         type-definition-part variable-declaration-part
//         procedure-and-function-declaration-part statement-part
auto parser::parse_block()
    -> parse_result<block> {
    auto result = block{};

    if (current_token().type() == token_type::keyword_label) {
        consume_and_advance();
        auto labels = label_declaration{};
        TRY(labels.labels, parse_group_of_symbols(
            *this,
            &parser::parse_identifier,
            {token_type::semicolon}, 
            token_type::comma
        ));
        result.label_declaration_part = std::move(labels);
    }

    if (current_token().type() == token_type::keyword_type) {

    }


    
}


// seperated into a function because it gets repetetive
// actual identifier "parsing" happens while lexing
// identifier = [a-zA-z][a-zA-Z0-9]*
auto parser::parse_identifier()
    -> parse_result<identifier> {
    if (m_lexer.lex_next_token().m_type != token_type::identifier) {
        return std::unexpected(parse_error{});
    }

    return identifier{consume_and_advance().m_view};
}

} // namespace fed 
