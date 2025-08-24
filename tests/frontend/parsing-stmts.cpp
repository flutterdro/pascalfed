#include "fed/diagnostics/buffer.hpp"
#include "fed/representations/ast/compare.hpp"
#include "fed/parser/parser.hpp"
#include "fed/representations/ast/nodes.hpp"
#include "fed/representations/ast/pretty-print.hpp"
#include <catch2/catch_test_macros.hpp>

#include "commons.hpp"

using namespace fed::ast;

TEST_CASE("Parsing unctx statements", "[parsing][frontend]") {
    auto diag   = fed::diagnostics_buffer();
    auto parser = fed::parser(""_fv, diag);
    auto ctx    = make_alphabet_context();

    SECTION("case statement") {
        auto source = 
            "case av of\n"
            "1, 3: ;\n"
            "2: ;\n"
            "end"_fv;
        parser.remount(source);
        auto expected = statement(case_statement{
            .case_index = variable('a'),
            .cases      = make_handle_list<case_t>(
                case_t{
                    .values = make_handle_list<constant>(
                        integer_literal{1}, integer_literal{3}
                    ),
                    .action = empty_statement(),
                },
                case_t{
                    .values = make_handle_list<constant>(
                        integer_literal(2)
                    ),
                    .action = empty_statement(),
                }
            )
        });
        auto result_exp = parser.parse_statement(ctx);
        diag.flush();
        if (not result_exp.has_value()) {
            FAIL(result_exp.error().message());
        }
        CHECK(expected == *result_exp);
    }
    SECTION("if statement") {
        auto source = 
            "if av then\n"
            "if bv then;"
            "else;"_fv;
        parser.remount(source);
        auto expected = statement(if_statement{
            .condition = variable('a'),
            .then_case = if_statement{
                .condition = variable('b'),
                .then_case = empty_statement(),
                .else_case = empty_statement(),
            }
        });
        auto result_exp = parser.parse_statement(ctx);
        diag.flush();
        if (not result_exp.has_value()) {
            FAIL(result_exp.error().message());
        }
        CHECK(expected == *result_exp);
    }
}
template<>
struct Catch::StringMaker<fed::ast::statement> {
    static auto convert(fed::ast::statement const& val)
        -> std::string { return fmt::format("{}", val); }
};
