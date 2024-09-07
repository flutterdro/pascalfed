#include "fed/parser/parser.hpp"
#include "fed/diagnostics/internal-error.hpp"
#include "fed/parser/parse_error.hpp"
#include "fed/representations/parse-tree.hpp"
#include "fed/scanner/token.hpp"
#include "fed/utils/superutil.hpp"

#include <boost/charconv/chars_format.hpp>
#include <boost/charconv/from_chars.hpp>

#include <algorithm>
#include <charconv>
#include <initializer_list>
#include <optional>
#include <type_traits>
#include <utility>


namespace fed {

// I kind of miss rust's operator ?
#define TRY(dest, exp) \
do {auto exp2 = exp;\
if (not exp2.has_value()) return std::unexpected{std::move(exp2.error())}; \
dest = std::move(exp2.value());} while(false)
#define TRY_OPT(opt) \
if (opt.has_value()) return std::unexpected{std::move(*opt)};

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

    auto const start = parser.cursor();

    {
        auto element = SymbolT{};
        TRY(element, (parser.*symbol_parse_function)());
        result.push_back(std::move(element));
    }

    while (stdr::none_of(terminals, func::equal_to(parser.current_token().type()))) {
        auto const end = parser.cursor();
        if (separator != token_type::empty and
            parser.consume_and_advance().type() != separator) {
            return std::unexpected(
                parse_error(
                    end.where(), source::view(start, end), 
                    parse_error::type::missing_token,
                    separator
                )
            );
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
    auto const program_view_start = cursor();

    TRY(result.head, parse_program_heading());
    
    auto const program_head_end = cursor();
    if (consume_and_advance().type() != token_type::semicolon) {
        return std::unexpected(
            parse_error(
                program_head_end.where(), 
                source::view(program_view_start, program_head_end),
                parse_error::type::missing_token,
                token_type::semicolon
            )
        );
    }
    
    TRY(result.body, parse_block());
    
    result.region = source::view(program_view_start, cursor());

    return result;
}

// program-heading = 'program' identifier ['(' identifier-list ')']
auto parser::parse_program_heading()
    -> parse_result<program_heading> {
    auto result = program_heading{};

    if (auto is_success = 
        try_consume_and_advance_expecting(token_type::keyword_program)) {
        return std::unexpected(*is_success);
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

    return result;
    
}

/// CONSTANT DECLARATION PARSING 

auto parser::parse_constant()
    -> parse_result<constant> {
    // if sign is specified then constant is treated as a signed number_integer
    auto result = constant();
    auto is_signed = 
        current_token().type() == token_type::minus;
    if (is_signed) consume_and_advance();
    
    auto const constant_token = current_token();
    // FUCK IT 
    // TODO: proper check for number validity
    if (constant_token.type() == token_type::number_integer) {
        auto num_view = constant_token.view();
        auto num = unsigned();
        std::from_chars(num_view.data(), num_view.data() + num_view.size(), num);
        if (is_signed) {
            result = -static_cast<int>(num);
        } else {
            result = num;
        }
    } else if (constant_token.type() == token_type::number_real) {
        auto num_view = constant_token.view();
        auto num = double();
        // I am losing my fucking mind
        // clang doesn't support fp from_chars 
        // IT IS IN GTEH FUCKIGN C++17. 
        // I had to pull boost in (frankly because I am too lazy to search 
        // for the alternatives but still...) 
        boost::charconv::from_chars(
            num_view.data(), num_view.data() + num_view.size(), 
            num, boost::charconv::chars_format::general
        );
        result = is_signed ? -num : num;
    } else if (constant_token.type() == token_type::identifier) {
        result = identifier(constant_token.view());
    } else if (constant_token.type() == token_type::literal) {
        result = constant_token
            .view()
            .subview(1, constant_token.view().size() - 2)
            .base();
    }

    return result;
}

/// TYPE DECLARATION PARSING

auto parser::parse_type_definition() 
    -> parse_result<type_definition> {
    auto result = type_definition();

    TRY(result.name, parse_identifier());
    TRY_OPT(try_consume_and_advance_expecting(token_type::equal));
    TRY(result.types, parse_type());

    return result;
}

auto parser::parse_type()
    -> parse_result<type> {
    return {};
}

auto parser::parse_array_types() 
    -> parse_result<array_type> {

}

auto parser::parse_enumerated_type()
    -> parse_result<enumerated_type> {
    auto result = enumerated_type();

    TRY_OPT(try_consume_and_advance_expecting(token_type::l_paren));
    TRY(result.identifiers, parse_group_of_symbols(
        *this, &parser::parse_identifier, 
        {token_type::r_paren}, token_type::comma)
    );
    TRY_OPT(try_consume_and_advance_expecting(token_type::r_paren));

    return result;
}

auto parser::parse_subrange_type()
    -> parse_result<subrange_type> {
    auto result = subrange_type();
}

auto parser::parse_procedure_declaration()
    -> parse_result<procedure_declaration> {
    using namespace std::literals;
    auto result = procedure_declaration();
    auto body = std::optional<block>();
    TRY(result.head, parse_procedure_heading());
    TRY_OPT(try_consume_and_advance_expecting(token_type::semicolon));

    
    if (current_token().view().base() != "forward"sv) {
        TRY(result.body, parse_block());
    } else {
        consume_and_advance();
    }
    TRY_OPT(try_consume_and_advance_expecting(token_type::semicolon));

    return result;
}


auto parser::parse_procedure_heading()
    -> parse_result<procedure_heading> {
    auto result = procedure_heading();
    TRY_OPT(try_consume_and_advance_expecting(token_type::keyword_procedure));
    TRY(result.name, parse_identifier());
    if (current_token().type() == token_type::l_paren) {
        TRY(result.formal_parametr_list, parse_formal_parameter_list());
    }
    TRY_OPT(try_consume_and_advance_expecting(token_type::r_paren));

    return result;
}

auto parser::parse_function_declaration()
    -> parse_result<function_declaration> {
    using namespace std::literals;
    auto result = function_declaration();
    auto body = std::optional<block>();
    TRY(result.head, parse_function_heading());
    TRY_OPT(try_consume_and_advance_expecting(token_type::semicolon));

    // TODO: forward declarations
    
    if (current_token().view().base() != "forward"sv) {
        TRY(result.body, parse_block());
    } else {
        consume_and_advance();
    }
    TRY_OPT(try_consume_and_advance_expecting(token_type::semicolon));

    return result;
}

auto parser::parse_function_heading()
    -> parse_result<function_heading> {
    auto result = function_heading();
    TRY_OPT(try_consume_and_advance_expecting(token_type::keyword_function));
    TRY(result.name, parse_identifier());
    if (current_token().type() == token_type::l_paren) {
        TRY(result.formal_parametr_list, parse_formal_parameter_list());
    }
    TRY_OPT(try_consume_and_advance_expecting(token_type::r_paren));
    TRY_OPT(try_consume_and_advance_expecting(token_type::colon));
    TRY(result.return_type, parse_identifier());

    return result;
}

auto parser::parse_formal_parameter_list()
    -> parse_result<group<formal_parameter>> {
    auto result = group<formal_parameter>();
    TRY_OPT(try_consume_and_advance_expecting(token_type::l_paren));
    TRY(result, parse_group_of_symbols(
        *this, &parser::parse_formal_parameter, 
        {token_type::r_paren}, token_type::semicolon));
    TRY_OPT(try_consume_and_advance_expecting(token_type::r_paren));

    return result;
}
auto parser::parse_formal_parameter()
    -> parse_result<formal_parameter> {
    auto result = formal_parameter();
    switch(current_token().type()) {
        using enum token_type;
        case keyword_function:  TRY(result, parse_function_heading()); break;
        case keyword_procedure: 
        default: TRY(result, parse_formal_parameter_simple()); break;
    }

    return result;
}
auto parser::parse_formal_parameter_simple() 
    -> parse_result<formal_parameter_simple> {
    auto result = formal_parameter_simple();
    result.is_variable = maybe_consume_and_advance_expecting(token_type::keyword_var);
    TRY(result.names, parse_group_of_symbols(
        *this, &parser::parse_identifier, 
        {token_type::colon}, token_type::comma) 
    );
    TRY_OPT(try_consume_and_advance_expecting(token_type::colon));
    TRY(result.type, parse_identifier());

    return result;
}
// seperated into a function because it gets repetetive
// actual identifier "parsing" happens while lexing
// identifier = [a-zA-z][a-zA-Z0-9]*
auto parser::parse_identifier()
    -> parse_result<identifier> {
    auto const start = cursor();
    if (current_token().type() != token_type::identifier) {
        return std::unexpected(
            parse_error(
                start.where(), current_token().view(),
                parse_error::type::unexpected_token,
                token_type::identifier
            )
        );
    }

    return identifier{consume_and_advance().m_view};
}

} // namespace fed 
