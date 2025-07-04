#include "fed/diagnostics/buffer.hpp"
#include "fed/diagnostics/internal-error.hpp"
#include "fed/parser/context.hpp"
#include "fed/parser/parser.hpp"
#include "fed/representations/ast.hpp"
#include "fed/representations/ast/forward.hpp"
#include "fed/representations/ast/nodes.hpp"
#include "fed/representations/ast/pretty-print.hpp"
#include "fed/representations/ast/compare.hpp"
#include "fed/representations/raw-source.hpp"
#include "fed/utils/superutil.hpp"

#include <catch2/catch_message.hpp>
#include <catch2/catch_test_macros.hpp>
#include <fmt/core.h>

#include "commons.hpp"

//

constexpr auto alphabet = []<std::size_t... Is>(std::index_sequence<Is...>) 
    -> std::array<fed::ast::variable_name, 26> {
    return {
        fed::ast::variable_name{
            .type = fed::poison_pill,
            .id = fed::ast::variable_id{1 + Is},
        }...
    };
}(std::make_index_sequence<26>());
auto make_binary_expression(fed::ast::binary_operation op, char lhs, char rhs) 
    -> fed::ast::expression {
    return fed::ast::binary_expression{
        .type = fed::poison_pill,
        .lhs  = fed::ast::expression(alphabet[lhs - 'a']),
        .rhs  = fed::ast::expression(alphabet[rhs - 'a']),
        .operation = op
    };
}
auto make_binary_expression(fed::ast::binary_operation op, 
                            fed::ast::expression lhs, 
                            char rhs) 
    -> fed::ast::expression {
    return fed::ast::binary_expression{
        .type = fed::poison_pill,
        .lhs  = std::move(lhs),
        .rhs  = fed::ast::expression(alphabet[rhs - 'a']),
        .operation = op
    };
}
auto make_binary_expression(fed::ast::binary_operation op, 
                            char lhs, 
                            fed::ast::expression rhs) 
    -> fed::ast::expression {
    return fed::ast::binary_expression{
        .type = fed::poison_pill,
        .lhs  = fed::ast::expression(alphabet[lhs - 'a']),
        .rhs  = std::move(rhs),
        .operation = op
    };
}
namespace fed::ast{
TEST_CASE("Parsing expression atom", "[frontend][parsing]"){
    using namespace fed;
    SECTION("Integer literal") {
        auto diagnostics = fed::diagnostics_buffer();
        auto ctx = make_alphabet_context();
        auto parser = fed::parser(
            fed::source::full_view("12312333"), 
            diagnostics
        );
        auto result = parser.parse_integer();
        if (not result.has_value()) {
            FAIL(result.error().message());
        }
        CHECK(result->value == 12312333);
    }
    SECTION("String literal") {

    }
}
TEST_CASE("Parsing binary expressions", "[frontend][parsing]") {
    using namespace fed;
    try {
        SECTION("Left-associativity") {
            auto diagnostics = fed::diagnostics_buffer();
            auto const ctx = make_alphabet_context();
            auto parser = fed::parser(
                fed::source::full_view("av + bv + cv + dv + ev"), 
                diagnostics
            );
            auto expected = make_binary_expression(ast::binary_operation::add, 'a', 'b');
            for (std::size_t i = 2 ; i < 5; ++i) {
                expected = make_binary_expression(
                    ast::binary_operation::add,
                    std::move(expected),
                    'a' + i
                );
            }
            if(auto result = parser.parse_expression(ctx)) {
                REQUIRE(*result == expected);
            } else {
                FAIL("failed to parse a valid expression");
            }
            
        }
        SECTION("Precedence") {
            auto diagnostics = fed::diagnostics_buffer();
            auto ctx = make_alphabet_context();
            auto parser = fed::parser(
                fed::source::full_view("fv >= av + bv * (cv + dv < ev) * kv <> jv"), 
                diagnostics
            );
            auto expected = 
                make_binary_expression(ast::binary_operation::not_equal,
                    make_binary_expression(ast::binary_operation::greater_or_equal, 
                        'f', 
                        make_binary_expression(ast::binary_operation::add,
                            'a',
                            make_binary_expression(ast::binary_operation::multiply,
                                make_binary_expression(ast::binary_operation::multiply, 
                                    'b',
                                    make_binary_expression(ast::binary_operation::less,
                                        make_binary_expression(ast::binary_operation::add, 'c', 'd'), 
                                        'e'
                                    )
                                ), 
                                'k'
                            )
                        )
                    ),
                    'j'
                );
            auto result = *parser.parse_expression(ctx);
            REQUIRE(result == expected);
        }
    } catch (fed::internal_error const& e) {
        fmt::println("{}", e);
        FAIL();
    } catch (std::exception const& e) {
        fmt::println("Usually this should not be possible but it happened:\n{}", e.what());
        FAIL();
    } 

}
}


template<>
struct Catch::StringMaker<fed::ast::expression> {
    static auto convert(fed::ast::expression const& val)
        -> std::string { return fmt::format("{}", val); }

};
