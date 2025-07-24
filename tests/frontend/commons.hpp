#ifndef FED_FRONTEND_TESTS_COMMONS_HPP_
#define FED_FRONTEND_TESTS_COMMONS_HPP_

#include "fed/diagnostics/internal-error.hpp"
#include "fed/parser/context.hpp"
#include "fed/representations/ast.hpp"
#include "fed/representations/ast/forward.hpp"
#include "fed/representations/ast/handle.hpp"
#include "fed/utils/macros.hpp"
#include "fed/utils/superutil.hpp"

using namespace std::literals;

consteval auto operator""_fv(char const* literal, std::size_t)
    -> fed::source::full_view {
    return {literal};
}

inline auto make_default_context()
    -> fed::semantic_context {
    return fed::semantic_context::make_global();
}

inline auto make_alphabet_context() 
    -> fed::semantic_context {
    auto context = fed::semantic_context::make_global();
    auto add_variable = [](fed::semantic_context& ctx, char c) {
        auto success = ctx.add_variable(fed::ast::variable_declaration{
            .name = ""s + c + 'v',
            .type = fed::poison_pill,
        });
        if (not success.has_value()) {
            throw fed::internal_error("alphabet context mishap");
        }
    };
    auto add_constant = [](fed::semantic_context& ctx, char c) {
        auto success = ctx.add_constant(fed::ast::constant_declaration{
            .name = ""s + c + 'c',
            .constant = fed::poison_pill,
        });
        if (not success.has_value()) {
            throw fed::internal_error("alphabet context mishap");
        }
    };
    for (auto c = 'a'; c <= 'z'; ++c) {
        add_variable(context, c);
        add_constant(context, c);
    }

    return context;
}

template<typename T>
inline constexpr auto make_handle_list =[](auto&&... args) {
    auto result = fed::ast::group<fed::ast::handle<T>>();
    (result.push_back(FWD(args)), ...);
    return result;
};

#endif
