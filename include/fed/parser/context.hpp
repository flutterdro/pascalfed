#ifndef FED_SEMANTIC_CONTEXT_HPP_
#define FED_SEMANTIC_CONTEXT_HPP_

#include "fed/representations/ast/forward.hpp"
#include "fed/representations/ast/handle.hpp"
#include "fed/representations/ast/name-scope.hpp"
#include "fed/representations/ast/sym-table.hpp"
#include "fed/parser/semantic-error.hpp"
#include <expected>
#include <memory>

namespace fed {

enum class check_result {
    success = 0,
    failure = 1,
};
template<typename T>
using semantic_result = std::expected<T, contextual_error>;
class semantic_context {
public:
    using type_observer = ast::observer_handle<ast::type>;
private:
    semantic_context(
        ast::symbol_table*, 
        std::unique_ptr<ast::name_scope>, 
        bool
    );
    
    template<typename Self>
    constexpr auto table(this Self&& self) 
        -> decltype(auto) {
        if (self.m_table_ptr == nullptr) {
            throw internal_error(
                "symbol_table pointer is null "
                "probably access to moved-from state"
            );
        }
        return *FWD(self).m_table_ptr;
    }
    template<typename Self>
    constexpr auto names(this Self&& self) 
        -> decltype(auto) {
        if (self.m_names_ptr == nullptr) {
            throw internal_error(
                "name_scope pointer is null "
                "probably access to moved-from state"
            );
        }
        return *FWD(self).m_names_ptr;
    }
public:
    semantic_context(semantic_context&&) noexcept;
    semantic_context(semantic_context const&) = delete;

    auto operator=(semantic_context&&) noexcept
        -> semantic_context&;
    auto operator=(semantic_context const&)
        -> semantic_context& = delete;

    ~semantic_context() noexcept;

    static auto make_global()
        -> semantic_context;
    auto make_local() const noexcept
        -> semantic_context;
private:
    auto init_poison_swamp()
        -> void;
    auto init_builtin_types() 
        -> void;
    auto init_builtin_constants()
        -> void;
public:

    auto enter_new_scope()
        -> void;
    auto exit_scope()
        -> void;

    auto add_type(ast::type_declaration)
        -> semantic_result<void>;
    auto add_function(ast::function_declaration)
        -> semantic_result<void>;
    auto add_variable(ast::variable_declaration)
        -> semantic_result<void>;
    auto add_constant(ast::constant_declaration)
        -> semantic_result<void>;

    auto get_poison_type() const 
        -> ast::type_identifier;
    
    auto get_ast_node(ast::function_id) const
        -> ast::observer_handle<ast::function_declaration>;
    auto get_ast_node(ast::variable_id) const
        -> ast::observer_handle<ast::variable_declaration>;
    auto get_ast_node(ast::constant_id) const
        -> ast::observer_handle<ast::constant_declaration>;
    auto get_ast_node(ast::type_id) const
        -> ast::observer_handle<ast::type_declaration>;

    auto try_get_function_id(ast::identifier_view) const
        -> ast::maybe<ast::function_id>;
    auto try_get_variable_id(ast::identifier_view) const
        -> ast::maybe<ast::variable_id>;
    auto try_get_type_id(ast::identifier_view) const
        -> ast::maybe<ast::type_id>;
    auto try_get_constant_id(ast::identifier_view) const
        -> ast::maybe<ast::constant_id>;

    auto type_from_id(ast::function_id) const
        -> type_observer;
    auto type_from_id(ast::variable_id) const
        -> type_observer;
    auto type_from_id(ast::type_id) const
        -> type_observer;
    auto type_from_id(ast::constant_id) const
        -> type_observer;

    auto get_integer_id() const 
        -> ast::type_id;
    auto get_real_id() const
        -> ast::type_id;
    auto get_bool_id() const
        -> ast::type_id;
    auto get_char_id() const
        -> ast::type_id;

    auto synthesize_dummy_expression() const
        -> ast::expression;

    auto match_types(type_observer, type_observer) const
        -> semantic_result<void>;
    auto is_ordinal(ast::type*)
        -> bool;
    auto get_expression_type(ast::expression const&) const
        -> ast::observer_handle<ast::type>;
    auto get_expression_type(ast::expression_leaf const&) const
        -> ast::observer_handle<ast::type>;
    auto get_expression_type(ast::observer_handle<ast::expression>) const
        -> ast::observer_handle<ast::type>;
    auto get_expression_type(ast::observer_handle<ast::expression_leaf>) const
        -> ast::observer_handle<ast::type>;
    auto get_constant_type(ast::constant const&) const
        -> ast::observer_handle<ast::type>;
    auto get_constant_type(ast::observer_handle<ast::constant>) const
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
    auto get_pointer_from_type(type_observer) const
        -> semantic_result<ast::observer_handle<ast::pointer_type>>;
    auto get_pointer_from_type(ast::type const& ptr) const 
        -> semantic_result<ast::observer_handle<ast::pointer_type>>;

    auto dereference_type(ast::observer_handle<ast::type>) const
        -> semantic_result<ast::observer_handle<ast::type>>;
    auto dereference_type(ast::observer_handle<ast::pointer_type>) const
        -> semantic_result<ast::observer_handle<ast::type>>;
    auto dereference_type(ast::pointer_type const&) const
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
    ast::symbol_table*               m_table_ptr;
    std::unique_ptr<ast::name_scope> m_names_ptr;
    bool                             m_is_global;
    // std::unique_ptr<scope> m_root;
    // scope* m_current_scope;
    // function_table m_functions;
    // variable_table m_variables;
    // constant_table m_constants;
    // type_table     m_types;
};
} // namespace fed
#endif
