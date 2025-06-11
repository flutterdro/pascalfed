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


//
auto create_test_context() {
    auto result = fed::semantic_context();
    auto add_variable = [&result](char letter) {
        auto err = result.add_variable({
            .name = fed::ast::identifier{letter, 'v'},
            .type = fed::poison_pill,
        });
        if (not err.has_value()) {
            throw fed::internal_error("error while creating test parse context");
        }
    };
    auto add_constant = [&result](char letter) {
        auto err = result.add_constant({
            .name = fed::ast::identifier{letter, 'c'},
            .constant = fed::poison_pill,
        });
        if (not err.has_value()) {
            throw fed::internal_error("error while creating test parse context");
        }
    };
    for (auto c = 'a'; c <= 'z'; ++c) {
        add_variable(c);
        add_constant(c);
    }
}
auto alphabet_context() 
    -> fed::semantic_context {
    auto context = fed::semantic_context();
    auto add_letter = [&context](char letter) {
        context.add_variable({ // NOLINT
            .name = fed::ast::identifier(1, letter),
            // type is poisoned to disable type checking
            .type = fed::poison_pill,
        });
    };
    for (auto c = 'a'; c <= 'z'; ++c) {
        add_letter(c);
    }
    return context;
}
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
        auto parser = fed::parser(
            fed::source::full_view("12312333"), 
            alphabet_context(), 
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
            auto parser = fed::parser(
                fed::source::full_view("a + b + c + d + e"), 
                alphabet_context(), 
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
            if(auto result = parser.parse_expression()) {
                REQUIRE(*result == expected);
            } else {
                FAIL("failed to parse a valid expression");
            }
            
        }
        SECTION("Precedence") {
            auto diagnostics = fed::diagnostics_buffer();
            auto parser = fed::parser(
                fed::source::full_view("f >= a + b * (c + d < e) * k <> j"), 
                alphabet_context(), 
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
            auto result = *parser.parse_expression();
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
