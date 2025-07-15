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
inline constexpr auto suck_error_in = [](parser& parser) {
    return [&](compilation_error err) { 
        return parser.push_error(std::move(err)); 
    };
};
inline constexpr auto contaminate = 
    []<typename T>(parser& parser, parse_result<T> patient0) 
        -> ast::handle<T> {
        return std::move(patient0)
            .transform_error(suck_error_in(parser))
            .transform(construct<ast::handle<T>>)
            .value_or(poison_pill);
    };
template<typename T> 
inline constexpr auto default_action = [](auto& parser) {
    return [&](auto&&... args) {
        return T(contaminate(parser, FWD(args))...);
    };
};
}
#endif
