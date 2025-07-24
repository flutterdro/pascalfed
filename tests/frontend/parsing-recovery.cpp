#include <catch2/catch_test_macros.hpp>

#include "commons.hpp"
#include "fed/diagnostics/buffer.hpp"
#include "fed/parser/parser.hpp"
#include "fed/parser/parser-helpers.hpp"
#include "fed/representations/ast/forward.hpp"
#include "fed/scanner/token.hpp"

inline constexpr auto simple_list_parse = 
    [](auto&& parser, auto&& context) {
        using t = fed::ast::group<fed::ast::handle<fed::ast::identifier>>;
        auto action = [](std::optional<t> res) { return std::move(*res); };
        auto id_list_parse = fed::make_some_parse(
            &fed::parser::parse_identifier, 
            fed::token_type::comma
        );
        using enum fed::token_type;
        return parser.chain_parse(
            context, action,
            l_paren, id_list_parse, r_paren
        );
    };

TEST_CASE("Some parse recovery") {
    auto diag   = fed::diagnostics_buffer();
    auto parser = fed::parser(""_fv, diag);
    auto ctx    = make_default_context();

    SECTION("missing separator recovery") {
        parser.remount("(a, a, a a, a)"_fv);
        auto result = simple_list_parse(parser, ctx);
        auto expected = make_handle_list<fed::ast::identifier>(
            "a"s, "a"s, "a"s, "a"s, "a"s
        );
        CHECK(std::ranges::equal(result, expected));
        CHECK(diag.current_error_count());
        diag.flush();
    }
    SECTION("wrong separator recovery") {
        parser.remount("(a, a, a < a, a)"_fv);
        auto result = simple_list_parse(parser, ctx);
        auto expected = make_handle_list<fed::ast::identifier>(
            "a"s, "a"s, "a"s, "a"s, "a"s
        );
        CHECK(std::ranges::equal(result, expected));
        CHECK(diag.current_error_count());
        diag.flush();
    }
}
