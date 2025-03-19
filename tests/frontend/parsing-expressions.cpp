#include "fed/diagnostics/buffer.hpp"
#include "fed/diagnostics/internal-error.hpp"
#include "fed/parser/context.hpp"
#include "fed/parser/parser.hpp"
#include "fed/representations/parse-tree.hpp"
#include "fed/representations/raw-source.hpp"
#include "fed/utils/superutil.hpp"

#include <catch2/catch_message.hpp>
#include <catch2/catch_test_macros.hpp>
#include <fmt/core.h>


//
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
namespace fed::ast{
template<typename T>
auto operator==(fed::ast::handle<T> const& lhs, fed::ast::handle<T> const& rhs)
    -> bool {
    // can't compare invalid nodes
    if (lhs.is_poisoned() and rhs.is_poisoned()) return true;
    if (lhs.is_poisoned() or rhs.is_poisoned()) return false;
    return *lhs == *rhs;
}
auto operator==(
    fed::ast::binary_expression const& lhs,
    fed::ast::binary_expression const& rhs
) -> bool;
auto operator==(
    fed::ast::unary_expression const& lhs,
    fed::ast::unary_expression const& rhs
) -> bool;
auto operator==(
    fed::ast::indexed_variable const& lhs,
    fed::ast::indexed_variable const& rhs
) -> bool;
auto operator==(
    fed::ast::called_variable const& lhs,
    fed::ast::called_variable const& rhs
) -> bool;
auto operator==(
    fed::ast::dereferenced_variable const& lhs,
    fed::ast::dereferenced_variable const& rhs
) -> bool;
auto operator==(
    fed::ast::membered_variable const& lhs,
    fed::ast::membered_variable const& rhs
) -> bool;
auto operator==(
    fed::ast::function_name const& lhs,
    fed::ast::function_name const& rhs
) -> bool;
auto operator==(
    fed::ast::variable_name const& lhs,
    fed::ast::variable_name const& rhs
) -> bool;
auto operator==(
    fed::ast::string_literal const&,
    fed::ast::string_literal const&
) -> bool;
auto operator==(
    fed::ast::integer_literal const&,
    fed::ast::integer_literal const&
) -> bool;
auto operator==(
    fed::ast::real_literal const&,
    fed::ast::real_literal const&
) -> bool;
auto operator==(
    fed::ast::enum_constant const&,
    fed::ast::enum_constant const&
) -> bool;
auto operator==(
    fed::ast::constant_name const&,
    fed::ast::constant_name const&
) -> bool;
}

TEST_CASE("Parsing binary expressions", "[frontend][parsing]") {
    using namespace fed;
    try {
    auto diagnostics = fed::diagnostics_buffer();
    auto parser = fed::parser(fed::source::full_view("f >= a + b * (c + d < e) * k <> j"), alphabet_context(), diagnostics);
    auto expected = fed::ast::expression(fed::ast::binary_expression{
        .type = poison_pill,
        .lhs = ast::expression(ast::variable_name{
            .type = poison_pill,
            .id = ast::variable_id{1},
        }),
        .rhs = ast::expression(ast::variable_name{
            .type = poison_pill,
            .id = ast::variable_id{3},
        }),
        .operation = ast::binary_operation::add,
    });
    auto result = *parser.parse_expression();
    REQUIRE(result == expected);
    } catch (fed::internal_error const& e) {
        fmt::println("{}", e);
        FAIL();
    }


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
namespace fed::ast {
auto operator==(
    fed::ast::binary_expression const& lhs,
    fed::ast::binary_expression const& rhs
) -> bool {
    return lhs.operation == rhs.operation and
           lhs.lhs == rhs.lhs and
           lhs.rhs == rhs.rhs;
}
auto operator==(
    fed::ast::unary_expression const& lhs,
    fed::ast::unary_expression const& rhs
) -> bool {
    return false;
}
auto operator==(
    fed::ast::indexed_variable const& lhs,
    fed::ast::indexed_variable const& rhs
) -> bool {
    return false;
}
auto operator==(
    fed::ast::called_variable const& lhs,
    fed::ast::called_variable const& rhs
) -> bool {
    return false;
}
auto operator==(
    fed::ast::membered_variable const& lhs,
    fed::ast::membered_variable const& rhs
) -> bool {
    return false;
}
auto operator==(
    fed::ast::function_name const& lhs,
    fed::ast::function_name const& rhs
) -> bool {
    return false;
}
auto operator==(
    fed::ast::variable_name const& lhs,
    fed::ast::variable_name const& rhs
) -> bool {
    return lhs.id == rhs.id;
}
auto operator==(
    real_literal const&, 
    real_literal const&
) -> bool {
    return false;
}
auto operator==(
    integer_literal const&, 
    integer_literal const&
) -> bool {
    return false;
}
auto operator==(
    constant_name const&, 
    constant_name const&
) -> bool {
    return false;
}
auto operator==(
    enum_constant const&, 
    enum_constant const&
) -> bool {
    return false;
}
auto operator==(
    string_literal const&, 
    string_literal const&
) -> bool {
    return false;
}
auto operator==(
    dereferenced_variable const&, 
    dereferenced_variable const&
) -> bool {
    return false;
}
}


template<>
struct Catch::StringMaker<fed::ast::expression> {
    static auto convert(fed::ast::expression const& val)
        -> std::string { return fmt::format("{}", val); }

};
