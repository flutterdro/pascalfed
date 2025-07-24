#ifndef FED_PARSER_MACROS_AKMSCOWMK
#define FED_PARSER_MACROS_AKMSCOWMK

#include "fed/parser/parser.hpp"

#define TRY_PARSE_TOKEN(token) do {\
auto is_succ = consume_and_advance_expecting(token);\
if (not is_succ) {\
    return std::unexpected(std::move(is_succ).error());\
}\
} while(false)

namespace fed {

template<typename T> 
inline constexpr auto default_action = [](auto&& parser) {
    return [&](auto&&... args) {
        return T(contaminate(parser, FWD(args))...);
    };
};

inline constexpr auto make_some_parse = [](auto&& parse_func, token_type separator) {
    using parse_res_t = ast::group<ast::handle<
        typename std::invoke_result_t<decltype(parse_func), parser&, semantic_context&>::value_type
    >>;
    return [parse_func_ = FWD(parse_func), separator](auto&& parser, auto&& ctx) {
        return parse_result<parse_res_t>(parser.some_parse(FWD(ctx), std::move(parse_func_), separator));
    };
};
inline constexpr auto make_many_parse = [](auto&& parse_func, token_type separator) {
    using parse_res_t = ast::group<ast::handle<
        typename std::invoke_result_t<decltype(parse_func), parser&, semantic_context&>::value_type
    >>;
    return [parse_func_ = FWD(parse_func), separator](auto&& parser, auto&& ctx) {
        return parse_result<parse_res_t>(parser.many_parse(FWD(ctx), std::move(parse_func_), separator));
    };
};
}
#endif
