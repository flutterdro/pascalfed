#include "fed/diagnostics/buffer.hpp"
#include "fed/parser/parser.hpp"
#include "fed/representations/parse-tree.hpp"
#include "fed/representations/raw-source.hpp"

#include <catch2/catch_message.hpp>
#include <catch2/catch_test_macros.hpp>
#include <fmt/core.h>


auto parse_expr(std::string_view expression) {
    auto buf = fed::diagnostics_buffer();
    auto parser = fed::parser(fed::source::full_view(expression), buf);
    return parser.parse_expression();
}

TEST_CASE("Parsing expressions", "[frontend][parsing]") {

    // auto expr_res  = parse_expr("a + b");
    // auto expr_res2 = parse_expr("a * b");
    // auto expr_res3 = parse_expr("a * (b + c)");
    // auto expr_res4 = parse_expr("(a + b) * c");
    // if (not expr_res.has_value()) FAIL("it should be a valid expression");
    // // else UNSCOPED_INFO("" << fmt::format("{}", *expr_res));
    // if (not expr_res2.has_value()) FAIL("it should be a valid expression");
    // // else UNSCOPED_INFO("" << fmt::format("{}", *expr_res2));
    // if (not expr_res3.has_value()) FAIL("it should be a valid expression");
    // // else UNSCOPED_INFO("" << fmt::format("{}", *expr_res3));
    // if (not expr_res4.has_value()) FAIL("it should be a valid expression");
    // else UNSCOPED_INFO("" << fmt::format("{}", *expr_res4));

    // FAIL("haya");
}

