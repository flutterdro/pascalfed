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

auto semantic_context::get_expression_type(ast::observer_handle<ast::expression> exp)
    -> ast::observer_handle<ast::type> {
    return exp.and_then(
        [this](ast::expression const& e) { return this->get_expression_type(e); }
    );
}
auto semantic_context::get_expression_type(ast::expression const& exp)
    -> ast::observer_handle<ast::type> {
    return std::visit(overloaded{
        [this](ast::expression_leaf const& e) { return this->get_expression_type(e); },
        [](auto const& e) -> ast::observer_handle<ast::type> { return e.type; }
    }, exp);
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
