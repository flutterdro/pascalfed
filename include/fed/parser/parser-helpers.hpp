#ifndef FED_PARSER_MACROS_AKMSCOWMK
#define FED_PARSER_MACROS_AKMSCOWMK

#include "fed/parser/context.hpp"
#include "fed/parser/parse_error.hpp"
#include "fed/parser/parser.hpp"
#include "fed/representations/ast/forward.hpp"

#define TRY_PARSE_TOKEN(token) do {\
auto is_succ = consume_and_advance_expecting(token);\
if (not is_succ) {\
    return std::unexpected(std::move(is_succ).error());\
}\
} while(false)

namespace fed {

inline constexpr auto monad_to_handle = 
    []<template<typename> typename M, typename T>(M<T> val) {
        return std::move(val)
            .transform(construct<ast::handle<T>>)
            .value_or(poison_pill);
    };
template<typename T> 
inline constexpr auto default_action =
    [](auto&&... args) {
        return T(monad_to_handle(FWD(args))...);
    };


inline constexpr auto make_some_parse = [](auto&& parse_func, token_type separator) {
    using parse_res_t = many_parse_result<ast::group<ast::handle<
        typename std::invoke_result_t<decltype(parse_func), parser&, semantic_context&>::value_type
    >>>;
    return [parse_func_ = FWD(parse_func), separator](auto&& parser, auto&& ctx) {
        return parser.some_parse(FWD(ctx), std::move(parse_func_), separator);
    };
};
inline constexpr auto make_many_parse = [](auto&& parse_func, token_type separator) {
    using parse_res_t = many_parse_result<ast::group<ast::handle<
        typename std::invoke_result_t<decltype(parse_func), parser&, semantic_context&>::value_type
    >>>;
    return [parse_func_ = FWD(parse_func), separator](auto&& parser, auto&& ctx) {
        return parser.many_parse(FWD(ctx), std::move(parse_func_), separator);
    };
};
inline constexpr auto make_chain_parse = [](auto&&... args) {
    return [...args_ = FWD(args)](parser& parser, uref<semantic_context> auto&& ctx) {
        return parser.chain_parse(ctx, args_...);
    };
};
template<typename T>
inline constexpr auto repopulate_identifiers = [](
    ast::handle_group<ast::identifier> identifiers,
    auto property
) {
    auto result = ast::handle_group<T>();
    for (auto i = std::size_t(0); i < identifiers.size(); ++i) {
        auto is_last_index = i == identifiers.size() - 1;
        auto plonk = [=, &property](ast::identifier&& idnt) {
            return T{
                std::move(idnt),
                is_last_index ? 
                    std::move(property) :
                    ast::clone(property)
            };
        };
        result.push_back(
            std::move(identifiers[i])
                .transform(plonk)
        );
    }

    return result;
};
}
#endif
