#ifndef FED_PARSER_HPP_
#define FED_PARSER_HPP_

#include "fed/diagnostics/buffer.hpp"
#include "fed/diagnostics/compile-error.hpp"
#include "fed/parser/context.hpp"
#include "fed/representations/ast/nodes.hpp"
#include "fed/representations/raw-source.hpp"
#include "fed/scanner/lex.hpp"
#include "fed/representations/ast.hpp"
#include "fed/parser/parse_error.hpp"
#include "fed/scanner/token.hpp"
#include "fed/utils/predicates.hpp"

#include <concepts>
#include <expected>
#include <fmt/base.h>
#include <initializer_list>
#include <utility>
#include <variant>
#include <vector>

namespace fed {

template<typename T>
using parse_result = std::expected<T, compilation_error>;

class parser {
    using token_stack = std::vector<token_type>;
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
    explicit parser(source::full_view, diagnostics_buffer&);

    auto remount(source::full_view)
        -> void;

    auto consume_and_advance()
        -> token_view;
    auto consume_and_advance_expecting(token_type token)
        -> parse_result<void>;
    auto advance_until(std::predicate<token_type> auto&& func)
        -> void;
    auto consume_and_advance_expecting(std::predicate<token_type> auto&& func)
        -> void;
    auto maybe_consume_and_advance_expecting(token_type token)
        -> bool;
    auto current_token()
        -> token_view;
    auto current_token_is(std::predicate<token_type> auto&& pred)
        -> bool { return pred(current_token().type()); }
    auto cursor() const noexcept
        -> source::iterator;
    auto diagnostics() noexcept
        -> diagnostics_buffer&;
    auto hazard_terminators() const noexcept
        -> token_stack const&;
    auto push_hazard_terminator(token_type)
        -> void;
    auto pop_hazard_terminator()
        -> void;
    auto push_error(compilation_error err) 
        -> std::monostate;

    // every parse function has a contract
    // they must advance lexer to the next token 
    // after parsed source
    
    struct parse_parameters {
        token_type                        separator = token_type::comma;
        std::initializer_list<token_type> success_terminators = {token_type::eof};
        std::initializer_list<token_type> hazard_terminators  = {token_type::eof};
    };
    template<typename F>
    using get_parse_invoke_t = std::invoke_result_t<F, parser, semantic_context&>::value_type;
    template<typename F, typename CtxT>
    auto parse_many(F&&, CtxT&&, parse_parameters const&)
        -> ast::group<ast::handle<get_parse_invoke_t<F>>> 
        requires std::same_as<std::remove_cvref_t<CtxT>, semantic_context>;
    template<typename... Ts>
    auto chain_parse(auto&& ctx, auto&& action, Ts&&... args);
    auto many_parse(auto&& ctx, auto&& parse_func, token_type separator, token_type end);
    auto some_parse(auto&& ctx, auto&& parse_func, token_type separator);

    auto parse_program() 
        -> parse_result<ast::program>; 
    auto parse_program_heading()
        -> parse_result<ast::handle<ast::program_heading>>;
    auto parse_block(semantic_context)
        -> parse_result<ast::block>;

    auto parse_type_definition(semantic_context& ctx)
        -> parse_result<ast::type_declaration>;
    auto parse_type_definitions(semantic_context& ctx)
        -> parse_result<void>;
    auto parse_variable_declaration()
        -> parse_result<ast::variable_declaration>;
    auto parse_variable_declarations()
        -> parse_result<void>;
    auto parse_type(semantic_context const&)
        -> parse_result<ast::type>;
    auto parse_type_identifier(semantic_context const&)
        -> parse_result<ast::type_identifier>;
    auto parse_pointer_type(semantic_context const&)
        -> parse_result<ast::pointer_type>;
    auto parse_enumerated_type(semantic_context const&)
        -> parse_result<ast::enumerated_type>;
    auto parse_subrange_type(semantic_context const&)
        -> parse_result<ast::subrange_type>;
    auto parse_array_type(semantic_context const&)
        -> parse_result<ast::array_type>;
    auto parse_function_type(semantic_context const&)
        -> parse_result<ast::function_type>;
    auto parse_argument(semantic_context const&)
        -> parse_result<ast::argument>;
    auto parse_set_type(semantic_context const&)
        -> parse_result<ast::set_type>;
    auto parse_file_type(semantic_context const&)
        -> parse_result<ast::file_type>;
    auto parse_record_type(semantic_context const&)
        -> parse_result<ast::record_type>;
    auto parse_fixed_part(semantic_context const&)
        -> parse_result<ast::fixed_part>;

    auto parse_constant(semantic_context const&)
        -> parse_result<ast::constant>;


    auto parse_identifier(semantic_context const&)
        -> parse_result<ast::identifier>;
    auto parse_function_declaration()
        -> parse_result<ast::handle<ast::function_declaration>>;
    auto parse_function_heading()
        -> parse_result<ast::handle<ast::function_heading>>;
    auto parse_procedure_declaration()
        -> parse_result<ast::handle<ast::procedure_declaration>>;
    auto parse_procedure_heading()
        -> parse_result<ast::handle<ast::procedure_heading>>;


    auto parse_expression(semantic_context const&, precedence::level = precedence::lowest)
        -> parse_result<ast::expression>;
    auto parse_binary_expression()
        -> parse_result<ast::binary_expression>;
    auto parse_unary_expression(semantic_context const&)
        -> parse_result<ast::unary_expression>;
    auto parse_expression_leaf(semantic_context const&)
        -> parse_result<ast::expression>;

    auto parse_integer()
        -> parse_result<ast::integer_literal>;
    auto parse_real()
        -> parse_result<ast::real_literal>;

private:
    auto determine_name_type(semantic_context const&, ast::identifier_view)
        -> ast::expression_atom;
    auto parse_expression_leaf(semantic_context const&, ast::expression)
        -> parse_result<ast::expression>;
    auto parse_call(semantic_context const&, ast::expression)
        -> parse_result<ast::expression>;
    auto parse_indexing(semantic_context const&, ast::expression)
        -> parse_result<ast::expression>;
    auto parse_dereferencing(semantic_context const&, ast::expression)
        -> parse_result<ast::expression>;
    auto parse_member_access(semantic_context const&, ast::expression)
        -> parse_result<ast::expression>;
    auto parse_expression(semantic_context const&, ast::expression lhs, precedence::level threshold)
        -> parse_result<ast::expression>;
    auto parse_lhs(semantic_context const&, precedence::level threshold)
        -> parse_result<ast::expression>;
    auto parse_rhs(semantic_context const&, ast::expression lhs, precedence::level threshold)
        -> parse_result<ast::expression>;
private:
    lexer m_lexer;
    token_stack m_hazard_terminators;
    diagnostics_buffer& m_diagnostics;
};

constexpr auto up(parser::precedence::level lvl) noexcept
    -> parser::precedence::level {
    return static_cast<parser::precedence::level>(lvl+1);
}

inline auto parser::advance_until(std::predicate<token_type> auto&& predicate)
    -> void {
    while (not (predicate and equal_to(token_type::eof))(current_token().type())) {
        consume_and_advance();
    }
}
inline auto parser::consume_and_advance_expecting(std::predicate<token_type> auto&& pred)
    -> void {
    if (pred(current_token().type())) {
        consume_and_advance();
        return;
    } else {
        push_error(parse_error());
    }
}

template<typename F, typename CtxT>
auto parser::parse_many(F&& parse_func, CtxT&& ctx, parse_parameters const& tokens)
    -> ast::group<ast::handle<get_parse_invoke_t<F>>> 
    requires std::same_as<std::remove_cvref_t<CtxT>, semantic_context> {
    auto result = ast::group<ast::handle<get_parse_invoke_t<F>>>();
    while (true) {
        if (current_token_is(any_of(tokens.success_terminators))) {
            break;
        }
        result.push_back(
            std::invoke(FWD(parse_func), *this, ctx)
                .transform_error(LIFT_MEMBER(push_error))
                .transform(construct<ast::handle<get_parse_invoke_t<F>>>)
                .value_or(poison_pill)
        );
        // auto in_need_of_recovery = not (
        //     equal_to(tokens.separator) or
        //     any_of(tokens.success_terminators) or 
        //     any_of(tokens.hazard_terminators) 
        // );
        // if (current_token_is(in_need_of_recovery)) {
        //     // unexpected tokens
        //     push_error(parse_error());
        //     advance_until(not in_need_of_recovery);
        // }
        
        if (current_token_is(any_of(tokens.hazard_terminators))) {
            // missin terminator 
            push_error(parse_error());
            break;
        }
        if (current_token_is(equal_to(tokens.separator))) {
            consume_and_advance();
            continue;
        }
    }

    return result;
} 
namespace detail {
template<auto...> struct any { constexpr explicit(false) any(auto&&...) noexcept {} };
template<std::size_t Index>
constexpr auto index_pack(auto&&... args) noexcept
    -> decltype(auto) {
    return [&]<std::size_t... Is>(std::index_sequence<Is...>) 
        -> decltype(auto) {
        return [](any<Is>&&..., auto&& nth, auto&&...) -> decltype(auto) {
            return FWD(nth);
        }(FWD(args)...);
    }(std::make_index_sequence<Index>());
}

template<std::size_t I, typename... Ts>
using index_pack_t = std::remove_cvref_t<decltype(
    index_pack<I>(std::declval<Ts>()...)
)>;

template<typename... Ts>
consteval auto count_anchors() -> std::size_t {
    return (std::same_as<Ts, token_type> + ...);
}
template<typename T1, typename T2, typename T3>
struct mapping {
    T1 anchors;
    T2 producers;
    T3 map_back;
};
template<typename... Ts>
consteval auto remap() {
    constexpr auto size = count_anchors<Ts...>();

    using anchors_t   = std::array<std::size_t, size>;
    using producers_t = std::array<std::size_t, sizeof...(Ts) - size>;
    using map_back_t  = std::array<std::size_t, sizeof...(Ts)>;

    auto maps = mapping<anchors_t, producers_t, map_back_t>();

    auto anchor_index   = std::size_t(0);
    auto producer_index = std::size_t(0);
    auto map_back_index = std::size_t(0);

    ([&]() {
        if (std::same_as<Ts, token_type>) {
            maps.anchors[anchor_index] = map_back_index;
            maps.map_back[map_back_index] = anchor_index;
            ++anchor_index;           
        } else {
            maps.producers[producer_index] = map_back_index;
            maps.map_back[map_back_index] = producer_index;
            ++producer_index;
        }
        ++map_back_index;
    }(), ...);

    return maps;
}

template<typename T>
using unwrap = std::invoke_result_t<T, parser, semantic_context&>;
}
template<typename... Ts>
auto parser::chain_parse(auto&& ctx, auto&& action, Ts&&... args) {
    static constexpr auto map = detail::remap<Ts...>();
    auto results = []<std::size_t... Is>(std::index_sequence<Is...>) {
        return std::tuple<detail::unwrap<
            detail::index_pack_t<map.producers[Is], Ts...>
        >...>();
    }(std::make_index_sequence<map.producers.size()>());
    [&]<std::size_t... Is>(std::index_sequence<Is...>){([&]{
        if constexpr (std::same_as<token_type, Ts>) {
            consume_and_advance_expecting(detail::index_pack<Is>(args...));
        } else {
            std::get<map.map_back[Is]>(results) = 
                std::invoke(detail::index_pack<Is>(args...), *this, ctx);
        }
    }(), ...);}(std::index_sequence_for<Ts...>());

    return std::apply([&](auto&&... args_) {
        return action(std::move(args_)...);
    }, std::move(results));
}

auto parser::some_parse(
    auto&& ctx, 
    auto&& parse_func, 
    token_type separator) {
    using parse_res_t = get_parse_invoke_t<decltype(parse_func)>;
    auto result = ast::group<ast::handle<parse_res_t>>();
    while (true) {
        result.push_back(
            std::invoke(FWD(parse_func), *this, ctx)
                .transform_error(LIFT_MEMBER(push_error))
                .transform(construct<ast::handle<parse_res_t>>)
                .value_or(poison_pill)
        );
        if (current_token_is(equal_to(separator))) {
            consume_and_advance();
            continue;
        } else { 
            break;
        }
    }

    return result;
} 
auto parser::many_parse(
    auto&& ctx, 
    auto&& parse_func,
    token_type separator,
    token_type end
) {
    if (current_token_is(equal_to(end))) {
        using parse_res_t = get_parse_invoke_t<decltype(parse_func)>;
        return ast::group<ast::handle<parse_res_t>>();
    }
    return some_parse(ctx, FWD(parse_func), separator);
}

} // namespace fed




#endif
