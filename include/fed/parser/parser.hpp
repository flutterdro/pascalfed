#ifndef FED_PARSER_HPP_
#define FED_PARSER_HPP_

#include "fed/diagnostics/buffer.hpp"
#include "fed/diagnostics/compile-error.hpp"
#include "fed/parser/anchor-stack.hpp"
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
#include <optional>
#include <utility>
#include <variant>
#include <vector>

namespace fed {

// template<typename T>
// using parse_result = std::expected<T, compilation_error>;

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
    // parsing mode in regrads to error recovery
    enum class mode {
        // it means default. but I used unchained because it sounds
        // more badass
        unchained,
        probing,
        contamination,
        // Oh Shit Oh God Oh Fuck mode. 
        // academics call it "panic".
        osogof,
    };
    enum class transit {
        
    };
public: 
    explicit parser(source::full_view, diagnostics_buffer&);

    auto remount(source::full_view)
        -> void;
    
    struct backup {
        lexer::backup bu;
    };
    auto preserve()
        -> backup;
    auto restore(backup)
        -> void;

    auto consume_and_advance()
        -> token_view;
    auto consume_and_advance_expecting(token_type token)
        -> parse_result<void>;
    auto breach_token_monitor(token_type token)
        -> void;
    auto advance_until(std::predicate<token_type> auto&& func)
        -> bool;
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
    template<typename Self>
    auto anchors(this Self&& self) noexcept
        -> decltype(auto) { return std::forward_like<Self>(self.m_anchors); }
    auto current_mode() const noexcept 
        -> mode { return m_mode; }
    auto hazard_terminators() const noexcept
        -> token_stack const&;
    auto push_hazard_terminator(token_type)
        -> void;
    auto pop_hazard_terminator()
        -> void;
    auto push_error(compilation_error err) 
        -> std::monostate;
    auto push_error(std::monostate)
        -> std::monostate { return {}; }

    // every parse function has a contract
    // they must advance lexer to the next token 
    // after parsed source
    
    struct parse_parameters {
        token_type                        separator = token_type::comma;
        std::initializer_list<token_type> success_terminators = {token_type::eof};
        std::initializer_list<token_type> hazard_terminators  = {token_type::eof};
    };
    template<typename F>
    using get_parse_invoke_t = std::invoke_result_t<F, parser&, semantic_context&>::value_type;
    template<typename F, typename CtxT>
    auto parse_many(F&&, CtxT&&, parse_parameters const&)
        -> ast::group<ast::handle<get_parse_invoke_t<F>>> 
        requires std::same_as<std::remove_cvref_t<CtxT>, semantic_context>;
    template<typename... Ts>
    auto chain_parse(auto&& ctx, auto&& action, Ts&&... args);
    auto many_parse(auto&& ctx, auto&& parse_func, token_type separator);
    auto some_parse(auto&& ctx, auto&& parse_func, token_type separator);
    auto breach_monitor_parse(auto&& ctx, auto&& parse_func);
    auto breach_monitor_parse(auto&& ctx, monitored auto&& parse_func);

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
    anchor_stack m_anchors;
    token_stack m_hazard_terminators;
    mode m_mode;
    diagnostics_buffer& m_diagnostics;
};
inline constexpr auto suck_error_in = [](parser& parser) {
    return [&](compilation_error err) { 
        return parser.push_error(std::move(err)); 
    };
};
inline constexpr auto contaminate = 
    []<typename T>(parser& parser, recovery_result<T> patient0) 
        -> ast::handle<T> {
        return std::move(patient0)
            .transform(construct<ast::handle<T>>)
            .value_or(poison_pill);
    };
constexpr auto up(parser::precedence::level lvl) noexcept
    -> parser::precedence::level {
    return static_cast<parser::precedence::level>(lvl+1);
}

inline auto parser::advance_until(std::predicate<token_type> auto&& predicate)
    -> bool {
    while (true) {
        if (current_token_is(predicate)) {
            return true;
        } 
        if (current_token_is(any_of(anchors()))) {
            return false;
        }
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

template<typename T>
inline constexpr bool is_anchor = 
    std::is_same_v<std::remove_cvref_t<T>, token_type>;
template<typename... Ts>
consteval auto count_anchors() -> std::size_t {
    return (is_anchor<Ts> + ...);
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
        if (is_anchor<Ts>) {
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
struct monitor_wrap;
template<typename T>
struct monitor_wrap<identity_monad<T>> { using type = identity_monad<T>; };
template<typename T>
struct monitor_wrap<recovery_result<T>> { using type = recovery_result<T>; };
template<typename T>
struct monitor_wrap<parse_result<T>> { using type = recovery_result<T>; };
template<typename T>
using monitor_wrap_t = typename monitor_wrap<T>::type;
template<typename T>
using unwrap = std::optional<monitor_wrap_t<get_parse_result<T>>>;
}
template<typename... Ts>
auto parser::chain_parse(auto&& ctx, auto&& action, Ts&&... args) {
    static constexpr auto map = detail::remap<Ts...>();
    auto results = []<std::size_t... Is>(std::index_sequence<Is...>) {
        return std::tuple<detail::unwrap<
            detail::index_pack_t<map.producers[Is], Ts...>
        >...>((static_cast<void>(Is), std::nullopt)...);
    }(std::make_index_sequence<map.producers.size()>());
    [&]<std::size_t... Is>(std::index_sequence<Is...>){
        anchors().push(std::array{
            detail::index_pack<map.anchors[map.anchors.size() - 1 - Is]>(args...)...
        });
    }(std::make_index_sequence<map.anchors.size()>());
    [&]<std::size_t... Is>(std::index_sequence<Is...>){([&]{
        if constexpr (detail::is_anchor<Ts>) {
            auto const& anchor = detail::index_pack<Is>(args...);
            breach_token_monitor(anchor);
            anchors().pop(anchor);
        } else {
            auto&& parse_func = detail::index_pack<Is>(FWD(args)...);
            std::get<map.map_back[Is]>(results) = breach_monitor_parse(ctx, parse_func);
        }
    }(), ...);}(std::index_sequence_for<Ts...>());

    return std::apply([&](auto&&... args_) {
        return action(*std::move(args_)...);
    }, std::move(results));
}

auto parser::some_parse(
    auto&& ctx, 
    auto&& parse_func, 
    token_type separator) {
    using parse_res_t = get_parse_invoke_t<decltype(parse_func)>;
    auto result = ast::handle_group<parse_res_t>();
    auto should_terminate =
        not equal_to(separator) and
        any_of(anchors());
    while (true) {
        if (current_mode() == mode::osogof) {
            break;
        }
        if (current_token_is(should_terminate)) break; 
        anchors().push(separator);
        result.push_back(
            breach_monitor_parse(ctx, parse_func)
                .transform(construct<ast::handle<parse_res_t>>)
                .value_or(poison_pill)
        );
        anchors().pop(separator);
        if (current_token_is(should_terminate)) { 
            break;
        }
        breach_token_monitor(separator);
    }

    return many_parse_result(std::move(result));
} 
auto parser::many_parse(
    auto&& ctx, 
    auto&& parse_func,
    token_type separator
) {
    // if (current_token_is(equal_to(anchors().top()))) {
    //     using parse_res_t = many_parse_result<
    //         ast::group<ast::handle<get_parse_invoke_t<decltype(parse_func)>>>>;
    //     return parse_res_t();
    // }
    return some_parse(ctx, FWD(parse_func), separator);
}

auto parser::breach_monitor_parse(auto&& ctx, auto&& parse_func) {
    using recovery_res_t = recovery_result<get_parse_invoke_t<decltype(parse_func)>>;
    switch (current_mode()) {
        case mode::unchained: {
            auto parse_res = std::invoke(parse_func, *this, ctx);
            if (parse_res.has_value()) {
                return recovery_res_t(std::move(*parse_res));
            } else {
                push_error(std::move(parse_res.error()));
                m_mode = mode::contamination;
                return recovery_res_t(recovery_fail);
            }
        }
        case mode::probing: {
            static constexpr auto max_probe_count = 2uz;
            auto backup = preserve();
            for (auto probe_attempt = 0uz; probe_attempt < max_probe_count; ++probe_attempt) {
                for (auto i = 0uz; i < probe_attempt; ++i) {
                    consume_and_advance();
                }
                if (current_token_is(any_of(anchors()))) {
                    m_mode = mode::osogof;
                    return recovery_res_t(recovery_fail);
                }
                auto probe_res = std::invoke(parse_func, *this, ctx);
                if (probe_res.has_value()) {
                    m_mode = mode::unchained;
                    return recovery_res_t(std::move(*probe_res));
                }
                restore(backup);
            }
            m_mode = mode::contamination;
            return recovery_res_t(recovery_fail);
        }
        case mode::contamination: 
        case mode::osogof: {
            return recovery_res_t(recovery_fail);
        }
    }
}
auto parser::breach_monitor_parse(
    auto&& ctx, 
    monitored auto&& parse_func
) {
    return std::invoke(FWD(parse_func), *this, ctx);
}

} // namespace fed




#endif
