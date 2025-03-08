#ifndef FED_PARSER_HPP_
#define FED_PARSER_HPP_

#include "fed/diagnostics/buffer.hpp"
#include "fed/parser/context.hpp"
#include "fed/representations/raw-source.hpp"
#include "fed/scanner/lex.hpp"
#include "fed/representations/parse-tree.hpp"
#include "fed/parser/parse_error.hpp"
#include "fed/scanner/token.hpp"

#include <concepts>
#include <expected>

namespace fed {

template<typename T>
using parse_result = std::expected<T, parse_error>;

class parser {
public:
    struct precedence {
        enum level {
            relational = 0,
            lowest = relational,
            adding,
            multiplying,
            highest = multiplying,
        };
    };
public: 
    explicit parser(source::full_view source, diagnostics_buffer& buffer);

    auto consume_and_advance()
        -> token_view;
    auto consume_and_advance_expecting(token_type token)
        -> std::optional<parse_error>;
    auto advance_until(std::predicate<token_type> auto&& func)
        -> void;
    auto consume_and_advance_expecting(std::predicate<token_type> auto&& func)
        -> std::optional<parse_error>;
    auto maybe_consume_and_advance_expecting(token_type token)
        -> bool;
    auto current_token()
        -> token_view;
    auto current_token_is(std::predicate<token_type> auto&& pred)
        -> bool;
    auto cursor() const noexcept
        -> source::iterator;
    auto context() noexcept
        -> semantic_context&;
    auto diagnostics() noexcept
        -> diagnostics_buffer&;

    // every parse function has a contract
    // they must advance lexer to the next token 
    // after parsed source

    auto parse_program() 
        -> parse_result<ast::handle<ast::program>>; 
    auto parse_program_heading()
        -> parse_result<ast::handle<ast::program_heading>>;
    auto parse_block()
        -> parse_result<ast::handle<ast::block>>;

    auto parse_type_definition()
        -> parse_result<ast::handle<ast::type_declaration>>;
    auto parse_variable_declaration()
        -> parse_result<ast::handle<ast::variable_declaration>>;
    auto parse_type()
        -> parse_result<ast::handle<ast::type>>;
    auto parse_enumerated_type()
        -> parse_result<ast::handle<ast::enumerated_type>>;
    auto parse_subrange_type()
        -> parse_result<ast::handle<ast::subrange_type>>;
    auto parse_array_type()
        -> parse_result<ast::handle<ast::array_type>>;
    auto parse_set_type()
        -> parse_result<ast::handle<ast::set_type>>;
    auto parse_file_type()
        -> parse_result<ast::handle<ast::file_type>>;
    auto parse_record_type()
        -> parse_result<ast::handle<ast::record_type>>;
    auto parse_field_list()
        -> parse_result<ast::handle<ast::record_type>>;
    auto parse_fixed_field()
        -> parse_result<ast::handle<ast::fixed_fields>>;
    auto parse_variant_part()
        -> parse_result<ast::handle<ast::variant_field>>;
    auto parse_variant()
        -> parse_result<ast::handle<ast::variant_part>>;

    auto parse_constant()
        -> parse_result<ast::handle<ast::constant>>;


    auto parse_identifier()
        -> parse_result<ast::identifier>;
    auto parse_formal_parameter_list()
        -> parse_result<ast::group<ast::handle<ast::formal_parameter>>>;
    auto parse_formal_parameter()
        -> parse_result<ast::handle<ast::formal_parameter>>;
    auto parse_formal_parameter_simple()
        -> parse_result<ast::handle<ast::formal_parameter_simple>>;
    auto parse_function_declaration()
        -> parse_result<ast::handle<ast::function_declaration>>;
    auto parse_function_heading()
        -> parse_result<ast::handle<ast::function_heading>>;
    auto parse_procedure_declaration()
        -> parse_result<ast::handle<ast::procedure_declaration>>;
    auto parse_procedure_heading()
        -> parse_result<ast::handle<ast::procedure_heading>>;


    auto parse_expression()
        -> parse_result<ast::expression>;
    auto parse_binary_expression()
        -> parse_result<ast::binary_expression>;
    auto parse_unary_expression()
        -> parse_result<ast::unary_expression>;
    auto parse_expression_leaf()
        -> parse_result<ast::expression_leaf>;

private:
    auto determine_name_type(ast::identifier_view)
        -> ast::bare_name;
    auto parse_expression_leaf(ast::expression_leaf)
        -> parse_result<ast::expression_leaf>;
    auto parse_call(ast::expression_leaf)
        -> parse_result<ast::expression_leaf>;
    auto parse_indexing(ast::expression_leaf)
        -> parse_result<ast::expression_leaf>;
    auto parse_dereferencing(ast::expression_leaf)
        -> parse_result<ast::expression_leaf>;
    auto parse_member_access(ast::expression_leaf)
        -> parse_result<ast::expression_leaf>;
    auto parse_expression(ast::expression lhs, precedence::level threshold)
        -> parse_result<ast::expression>;
    auto parse_lhs(precedence::level threshold)
        -> parse_result<ast::expression>;
    auto parse_rhs(ast::expression lhs, precedence::level threshold)
        -> parse_result<ast::binary_expression>;
private:
    lexer m_lexer;
    diagnostics_buffer& m_diagnostics;
    semantic_context m_context;
};





} // namespace fed



#endif
