#include "fed/parser/context.hpp"
#include "fed/parser/semantic-error.hpp"
#include "fed/representations/parse-tree.hpp"
#include "fed/representations/symbol-table.hpp"
#include "fed/utils/superutil.hpp"
#include <__expected/unexpected.h>
#include <algorithm>
#include <array>
#include <functional>
#include <memory>
#include <optional>
#include <span>
#include <string_view>


namespace fed {

namespace {
struct type_matcher {
    auto match(
        ast::observer_handle<ast::type> type1,
        ast::observer_handle<ast::type> type2
    ) const -> semantic_result<void> {
        if (type1.is_poisoned() or type2.is_poisoned()) return {};
        return (*this)(type1.unsafe_value(), type2.unsafe_value());
    }

    auto operator()(ast::type const& type1, ast::type const& type2) const
        -> semantic_result<void> { return std::visit(*this, type1, type2); }
    auto operator()(auto const& type1, ast::type const& type2) const 
        -> semantic_result<void> { return (*this)(type2, type1); }
    auto operator()(ast::type const& type1, auto const& type2) const
        -> semantic_result<void> { 
        return std::visit(
            [&type2, this](auto const& type1_) { return (*this)(type1_, type2); },
            type1
        );
    }

    auto operator()(ast::type_identifier const& type1, ast::type_identifier const& type2) const 
        -> semantic_result<void> {
        if (type1.id == type2.id) return {};
        else return std::unexpected(contextual_error());
    }
    auto operator()(ast::type_identifier const& type1, auto const& type2) const 
        -> semantic_result<void> {
        auto underlying_type_obs = ctx.type_from_id(type1.id);
        if (underlying_type_obs.is_poisoned()) return {};
        else return (*this)(underlying_type_obs.unsafe_value(), type2);
    }
    auto operator()(auto const& type1, ast::type_identifier const& type2) const
        -> semantic_result<void> {
        return (*this)(type2, type1);
    }
    auto operator()(ast::array_type const& type1, ast::array_type const& type2) const
        -> semantic_result<void> {
        if (not match(type1.component_type, type2.component_type).has_value()) {
            return std::unexpected(contextual_error());
        }
        if (type1.index_types.size() != type2.index_types.size()) {
            return std::unexpected(contextual_error());
        }
        for (std::size_t i = 0; i < type1.index_types.size(); ++i) {
            if (not match(type1.index_types[i], type2.index_types[i]).has_value()) {
                return std::unexpected(contextual_error());
            }
        }
        return {};
    }
    auto operator()(ast::set_type const& type1, ast::set_type const& type2) const
        -> semantic_result<void> {
        auto underlying_type_obs1 = ast::observer_handle(type1.base);
        auto underlying_type_obs2 = ast::observer_handle(type2.base);
        return match(underlying_type_obs1, underlying_type_obs2);
    }
    auto operator()(ast::function_type const& type1, ast::function_type const& type2) const
        -> semantic_result<void> {
        if (not match(type1.return_type, type2.return_type).has_value()) {
            return std::unexpected(contextual_error());
        }
        if (type1.argument_types.size() != type2.argument_types.size()) {
            return std::unexpected(contextual_error());
        }
        for (std::size_t i = 0; i < type1.argument_types.size(); ++i) {
            if (not match(type1.argument_types[i], type2.argument_types[i]).has_value()) {
                return std::unexpected(contextual_error());
            }
        }
        return {};
    }
    auto operator()(ast::procedure_type const& type1, ast::procedure_type const& type2) const
        -> semantic_result<void> {
        if (type1.argument_types.size() != type2.argument_types.size()) {
            return std::unexpected(contextual_error());
        }
        for (std::size_t i = 0; i < type1.argument_types.size(); ++i) {
            if (not match(type1.argument_types[i], type2.argument_types[i]).has_value()) {
                return std::unexpected(contextual_error());
            }
        }
        return {};
    }
    auto operator()(ast::subrange_type const& type1, ast::subrange_type const& type2) const
        -> semantic_result<void> {
        
    }

    //catch all
    auto operator()(auto const& type1, auto const& type2) const
        -> semantic_result<void> { return std::unexpected(contextual_error()); }


    semantic_context const& ctx;
};
}


auto semantic_context::match_types(type_observer type1,  type_observer type2) const 
    -> semantic_result<void> {
    return type_matcher{*this}.match(type1, type2);
}


//
/// Named lookup
//

auto semantic_context::try_get_variable_id(ast::identifier_view name) const
    -> ast::maybe<variable_id> {
    return m_current_scope->lookup(name).and_then([](symbol sym) 
        -> ast::maybe<variable_id> {
        if (sym.type != symbol_type::variable) return std::nullopt;
        return static_cast<variable_id>(sym.id);
    });
}
auto semantic_context::try_get_function_id(ast::identifier_view name) const
    -> ast::maybe<function_id> {
    return m_current_scope->lookup(name).and_then([](symbol sym) 
        -> ast::maybe<function_id> {
        if (sym.type != symbol_type::function) return std::nullopt;
        return static_cast<function_id>(sym.id);
    });
}
auto semantic_context::try_get_constant_id(ast::identifier_view name) const
    -> ast::maybe<constant_id> {
    return m_current_scope->lookup(name).and_then([](symbol sym) 
        -> ast::maybe<constant_id> {
        if (sym.type != symbol_type::constant) return std::nullopt;
        return static_cast<constant_id>(sym.id);
    });
}
auto semantic_context::try_get_enum_id(ast::identifier_view name) const
    -> ast::maybe<enum_id> {
    return m_current_scope->lookup(name).and_then([](symbol sym) 
        -> ast::maybe<enum_id> {
        if (sym.type != symbol_type::enum_) return std::nullopt;
        return static_cast<enum_id>(sym.id);
    });
}


auto semantic_context::get_ast_node(variable_id id) const
    -> ast::observer_handle<ast::variable_declaration> {
    return m_variables.lookup(id);
}
auto semantic_context::get_ast_node(function_id id) const 
    -> ast::observer_handle<ast::function_declaration> {
    return m_functions.lookup(id);
}
auto semantic_context::get_ast_node(constant_id id) const 
    -> ast::observer_handle<ast::constant_declaration> {
    return m_constants.lookup(id);
}
auto semantic_context::get_ast_node(enum_id id) const 
    -> ast::observer_handle<ast::type> {
    return m_enums.lookup(id);
}
auto semantic_context::get_ast_node(type_id id) const
    -> ast::observer_handle<ast::type_declaration> {
    return m_types.lookup(id);
}

auto semantic_context::type_from_id(variable_id id) const
    -> type_observer {
    return get_ast_node(id).and_then(
        [](ast::variable_declaration const& decl) -> type_observer { return decl.type; }
    );
}
auto semantic_context::type_from_id(constant_id id) const
    -> type_observer {
    return get_ast_node(id).and_then(
        [&](ast::constant_declaration const& decl) -> type_observer { 
            return get_constant_type(decl.constants);
        }
    );
}
auto semantic_context::type_from_id(function_id id) const
    -> type_observer {
    return get_ast_node(id).and_then(
        [](ast::function_declaration const& decl) -> type_observer { return decl.type; }
    );
}
auto semantic_context::type_from_id(enum_id id) const
    -> type_observer {
    return get_ast_node(id);
}
auto semantic_context::type_from_id(type_id id) const
    -> type_observer {
    return get_ast_node(id).and_then(
        [](ast::type_declaration const& decl) -> type_observer { return decl.types; }
    );
}


auto semantic_context::get_expression_type(ast::observer_handle<ast::expression> exp) const
    -> ast::observer_handle<ast::type> {
    return exp.and_then(
        [this](ast::expression const& e) { return this->get_expression_type(e); }
    );
}
auto semantic_context::get_expression_type(ast::observer_handle<ast::expression_leaf> exp) const
    -> type_observer { return exp.and_then(LIFT_MEMBER(get_expression_type)); }
auto semantic_context::get_expression_type(ast::expression_leaf const& exp) const 
    -> type_observer {
    return std::visit(overloaded{
            [&](ast::bare_name const& name) { 
                return std::visit(overloaded{
                    [](auto const& name_) { return name_.type; },
                    [&](ast::constant const& name_) { return get_constant_type(name_); }
                }, name);
            },
            [](auto const& expr) { return expr.type; }
        }, exp
    );
}
auto semantic_context::get_expression_type(ast::expression const& exp) const
    -> ast::observer_handle<ast::type> {
    return std::visit(overloaded{
        [this](ast::expression_leaf const& e) { return this->get_expression_type(e); },
        [](auto const& e) -> ast::observer_handle<ast::type> { return e.type; }
    }, exp);
}
auto semantic_context::get_constant_type(ast::constant const& cnst) const
    -> type_observer {
    // TODO: proper types 
    // can't do now since havent introduced Int and Real in the symbol-table
    return poison_pill;
}
auto semantic_context::get_constant_type(ast::observer_handle<ast::constant> cnst) const
    -> type_observer {
    return cnst.and_then(LIFT_MEMBER(get_constant_type));
}
auto semantic_context::get_function_from_type(type_observer function) const
    -> semantic_result<ast::observer_handle<ast::function_type>> {
    return function
        .and_then(LIFT_MEMBER(get_function_from_type));
}
auto semantic_context::get_function_from_type(ast::type const& function) const
    -> semantic_result<ast::observer_handle<ast::function_type>> {
    auto maybe_function_type = std::get_if<ast::function_type>(std::addressof(function));
    if (maybe_function_type == nullptr) {
        return std::unexpected(contextual_error());
    } else {
        return ast::observer_handle(maybe_function_type);
    }
}

auto semantic_context::call_type(
    type_observer type, 
    std::span<type_observer> caller_args
) const -> semantic_result<type_observer> {
    return get_function_from_type(type)
        .and_then(std::bind_back(LIFT_MEMBER(call_type), caller_args));
}
auto semantic_context::call_type(
    ast::observer_handle<ast::function_type> func,
    std::span<type_observer> caller_args
) const -> semantic_result<type_observer> {
    return func
        .and_then(std::bind_back(LIFT_MEMBER(call_type), caller_args));
}
auto semantic_context::call_type(
    ast::function_type const& func, 
    std::span<type_observer> caller_args
) const -> semantic_result<type_observer> {
    auto const& callee_args = func.argument_types;
    if (callee_args.size() != caller_args.size()) {
        // TODO: errors
        return std::unexpected(contextual_error());
    }
    for (std::size_t i = 0; i < callee_args.size(); ++i) {
        if (auto check_result = match_types(callee_args[i], caller_args[i])) {
            continue;
        } else {
            return std::unexpected(check_result.error());
        }
    }
    return func.return_type;
}
auto semantic_context::get_array_from_type(type_observer array) const
    -> semantic_result<ast::observer_handle<ast::array_type>> {
    return array
        .and_then(LIFT_MEMBER(get_array_from_type));
}
auto semantic_context::get_array_from_type(ast::type const& array) const
    -> semantic_result<ast::observer_handle<ast::array_type>> {
    auto maybe_array_type = std::get_if<ast::array_type>(std::addressof(array));
    if (maybe_array_type == nullptr) {
        return std::unexpected(contextual_error());
    } else {
        return ast::observer_handle(maybe_array_type);
    }
}

auto semantic_context::index_type(
    type_observer type, 
    std::span<type_observer> indexer_args
) const -> semantic_result<type_observer> {
    return get_array_from_type(type)
        .and_then(std::bind_back(LIFT_MEMBER(index_type), indexer_args));
}
auto semantic_context::index_type(
    ast::observer_handle<ast::array_type> arr,
    std::span<type_observer> indexer_args
) const -> semantic_result<type_observer> {
    return arr
        .and_then(std::bind_back(LIFT_MEMBER(index_type), indexer_args));
}
auto semantic_context::index_type(
    ast::array_type const& arr, 
    std::span<type_observer> indexer_args
) const -> semantic_result<type_observer> {
    auto const& indexee_args = arr.index_types;
    if (indexee_args.size() != indexer_args.size()) {
        // TODO: errors
        return std::unexpected(contextual_error());
    }
    for (std::size_t i = 0; i < indexee_args.size(); ++i) {
        if (auto check_result = match_types(indexee_args[i], indexer_args[i])) {
            continue;
        } else {
            return std::unexpected(check_result.error());
        }
    }
    return arr.component_type;
}

auto semantic_context::get_record_from_type(type_observer type) const
    -> semantic_result<ast::observer_handle<ast::record_type>> {
    return type.and_then(LIFT_MEMBER(get_record_from_type));
}
auto semantic_context::get_record_from_type(ast::type const& record) const
    -> semantic_result<ast::observer_handle<ast::record_type>> {
    auto maybe_record_type = std::get_if<ast::record_type>(&record);
    if (maybe_record_type == nullptr) {
        return std::unexpected(contextual_error());
    } else {
        return maybe_record_type;
    }
}

auto semantic_context::member_type(
    type_observer type,
    ast::identifier_view name
) const -> semantic_result<type_observer> {
    return get_record_from_type(type).and_then(
        [=, this](auto const& record) {
            return this->member_type(record, name);
        }
    );
}
auto semantic_context::member_type(
    ast::observer_handle<ast::record_type> record_obs,
    ast::identifier_view name
) const -> semantic_result<type_observer> {
    return record_obs.and_then([=, this](auto const& record) {
        return this->member_type(record, name);
    });
}
auto semantic_context::member_type(
    ast::record_type const& record, 
    ast::identifier_view name
) const -> semantic_result<type_observer> {
    auto const& member_map = record.fixed_part.members;
    auto const member_it = member_map.find(name);
    if (member_it == member_map.end()) {
        return std::unexpected(contextual_error());
    } 
    return member_it->second;
}
auto semantic_context::get_pointer_from_type(type_observer type) const
    -> semantic_result<ast::observer_handle<ast::pointer_type>> {
    return type.and_then(LIFT_MEMBER(get_pointer_from_type));
}
auto semantic_context::get_pointer_from_type(ast::type const& pointer) const
    -> semantic_result<ast::observer_handle<ast::pointer_type>> {
    auto maybe_pointer_type = std::get_if<ast::pointer_type>(&pointer);
    if (maybe_pointer_type == nullptr) {
        return std::unexpected(contextual_error());
    } else {
        return maybe_pointer_type;
    }
}
auto semantic_context::dereference_type(type_observer type) const 
    -> semantic_result<type_observer> {
    return get_pointer_from_type(type).and_then(LIFT_MEMBER(dereference_type));
}
auto semantic_context::dereference_type(ast::observer_handle<ast::pointer_type> ptr) const
    -> semantic_result<type_observer> {
    return ptr.and_then(LIFT_MEMBER(dereference_type));
}
auto semantic_context::dereference_type(ast::pointer_type const& ptr) const
    -> semantic_result<type_observer> {
    return ptr.base;
}
auto semantic_context::add_types(type_observer lhs, type_observer rhs) const
    -> semantic_result<type_observer> {
    return ast::then_all(
        LIFT_MEMBER(add_types), 
        lhs, rhs
    );
}

struct type_adder {
    auto operator()(auto const&, auto const&)
        -> semantic_result<ast::observer_handle<ast::type>> {
        return std::unexpected(contextual_error());
    }
    semantic_context const& ctx;
};

auto semantic_context::add_types(ast::type const& lhs, ast::type const& rhs) const
    -> semantic_result<type_observer> {
    return std::visit(type_adder{*this}, lhs, rhs);
}

} // namespace fed
