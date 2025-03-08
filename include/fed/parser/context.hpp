#ifndef FED_SEMANTIC_CONTEXT_HPP_
#define FED_SEMANTIC_CONTEXT_HPP_

#include "fed/representations/parse-tree.hpp"
#include "fed/representations/symbol-table.hpp"
#include "fed/parser/semantic-error.hpp"
#include <expected>

namespace fed {

enum class check_result {
    success = 0,
    failure = 1,
};
template<typename T>
using semantic_result = std::expected<T, contextual_error>;
class semantic_context {
    using function_table = symbol_mapback<ast::observer_handle<ast::function_declaration>>;
    using variable_table = symbol_mapback<ast::observer_handle<ast::variable_declaration>>;
    using constant_table = symbol_mapback<ast::observer_handle<ast::constant_declaration>>;
    using type_table     = symbol_mapback<ast::observer_handle<ast::type_declaration>>;
    using enum_table     = symbol_mapback<ast::observer_handle<ast::enumerated_type>>;
public:
    using function_id = function_table::id;
    using variable_id = variable_table::id;
    using constant_id = constant_table::id;
    using type_id     = type_table::id;
    using enum_id     = enum_table::id;

    using type_observer = ast::observer_handle<ast::type>;

    auto enter_new_scope()
        -> void;
    auto exit_scope()
        -> void;

    auto add_type(ast::type_declaration_handle&)
        -> semantic_result<void>;
    auto add_function(ast::function_declaration_handle&)
        -> semantic_result<void>;
    auto add_variable(ast::variable_declaration_handle&)
        -> semantic_result<void>;
    auto add_constant(ast::constant_declaration_handle&)
        -> semantic_result<void>;
    
    auto get_ast_node(function_id)
        -> ast::function_declaration*;
    auto get_ast_node(variable_id)
        -> ast::variable_declaration*;
    auto get_ast_node(constant_id)
        -> ast::constant_declaration*;
    auto get_ast_node(type_id)
        -> ast::type_declaration*;

    auto try_get_function_id(ast::identifier_view) const
        -> ast::maybe<function_id>;
    auto try_get_variable_id(ast::identifier_view) const
        -> ast::maybe<variable_id>;
    auto try_get_type_id(ast::identifier_view) const
        -> ast::maybe<type_id>;
    auto try_get_constant_id(ast::identifier_view) const
        -> ast::maybe<constant_id>;
    auto try_get_enum_id(ast::identifier_view) const
        -> ast::maybe<enum_id>;

    auto type_from_id(function_id) const
        -> type_observer;
    auto type_from_id(variable_id) const
        -> type_observer;
    auto type_from_id(type_id) const
        -> type_observer;
    auto type_from_id(constant_id) const
        -> type_observer;
    auto type_from_id(enum_id) const
        -> type_observer;

    auto match_types(type_observer, type_observer) const
        -> semantic_result<void>;
    auto is_ordinal(ast::type*)
        -> bool;
    auto get_expression_type(ast::expression const&)
        -> ast::observer_handle<ast::type>;
    auto get_expression_type(ast::expression_leaf const&)
        -> ast::observer_handle<ast::type>;
    auto get_expression_type(ast::observer_handle<ast::expression>)
        -> ast::observer_handle<ast::type>;
    auto get_expression_type(ast::observer_handle<ast::expression_leaf>)
        -> ast::observer_handle<ast::type>;
    auto get_function_from_type(type_observer function) const
        -> semantic_result<ast::observer_handle<ast::function_type>>;
    auto get_function_from_type(ast::type const& function) const
        -> semantic_result<ast::observer_handle<ast::function_type>>;
    auto get_array_from_type(type_observer array) const
        -> semantic_result<ast::observer_handle<ast::array_type>>;
    auto get_array_from_type(ast::type const& array) const
        -> semantic_result<ast::observer_handle<ast::array_type>>;
    auto get_record_from_type(type_observer) const 
        -> semantic_result<ast::observer_handle<ast::record_type>>;
    auto get_record_from_type(ast::type const& record) const
        -> semantic_result<ast::observer_handle<ast::record_type>>;

    auto dereference_type(ast::observer_handle<ast::type>)
        -> semantic_result<ast::observer_handle<ast::type>>;
    auto dereference_type(ast::type const&)
        -> semantic_result<ast::observer_handle<ast::type>>;
    auto index_type(type_observer function, std::span<type_observer> arguments) const
        -> semantic_result<type_observer>;
    auto index_type(ast::observer_handle<ast::array_type>, std::span<type_observer>) const 
        -> semantic_result<type_observer>; 
    auto index_type(ast::array_type const& array, std::span<type_observer> arguments) const
        -> semantic_result<type_observer>;
    auto call_type(type_observer function, std::span<type_observer> arguments) const
        -> semantic_result<type_observer>;
    auto call_type(ast::observer_handle<ast::function_type>, std::span<type_observer>) const 
        -> semantic_result<type_observer>; 
    auto call_type(ast::function_type const& function, std::span<type_observer> arguments) const
        -> semantic_result<type_observer>;
    auto member_type(ast::record_type const& record, ast::identifier_view) const 
        -> semantic_result<type_observer>;
    auto member_type(ast::observer_handle<ast::record_type>, ast::identifier_view) const 
        -> semantic_result<type_observer>;
    auto member_type(type_observer, ast::identifier_view) const 
        -> semantic_result<type_observer>;

    auto add_types(type_observer, type_observer) const
        -> semantic_result<type_observer>;
    auto add_types(ast::type const&, ast::type const&) const
        -> semantic_result<type_observer>;
    auto sub_types(type_observer, type_observer) const
        -> semantic_result<type_observer>;
    auto sub_types(ast::type const&, ast::type const&) const
        -> semantic_result<type_observer>;
private:
    scope* m_current_scope;
    function_table m_functions;
    variable_table m_variables;
    constant_table m_constants;
    type_table     m_types;
};
} // namespace fed
#endif
