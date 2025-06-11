#include "fed/diagnostics/buffer.hpp"
#include "fed/parser/parser.hpp"
#include "fed/parser/context.hpp"
#include "fed/representations/ast/nodes.hpp"
#include "fed/representations/raw-source.hpp"
#include "fed/representations/ast.hpp"
#include "fed/representations/ast/compare.hpp"
#include "fed/representations/ast/pretty-print.hpp"
#include <catch2/catch_test_macros.hpp>

consteval auto operator""_fv(char const* literal, std::size_t)
    -> fed::source::full_view {
    return {literal};
}

struct parse_type_case {
    fed::source::full_view source;
    fed::ast::type         expected_result;
};
template<typename...>
struct first_t;
template<typename T, typename... Ts>
struct first_t<T, Ts...> { using type = T; };
template<typename... Ts>
using first = typename first_t<Ts...>::type;
TEST_CASE("Parsing type declarations", "[frontend][parsing]") {
    using namespace fed;
    auto diagnostics = fed::diagnostics_buffer();
    auto context     = fed::semantic_context();
    auto int_ident   = fed::ast::type_identifier(context.get_integer_id());
    auto int_type_handle = [&](){ return ast::handle(ast::type(auto(int_ident))); };
    auto int_type = [&](){ return fed::ast::type(auto(int_ident)); };
    auto real_ident  = fed::ast::type_identifier(context.get_real_id());
    auto real_type    = [&](){ return fed::ast::type(auto(real_ident)); };
    auto real_type_handle = [&](){ return ast::handle(ast::type(auto(int_ident))); };
    auto bool_ident  = fed::ast::type_identifier(context.get_bool_id());
    auto bool_type    = [&](){ return fed::ast::type(auto(bool_ident)); };
    auto bool_type_handle = [&](){ return ast::handle(ast::type(auto(bool_ident))); };
    auto char_ident  = fed::ast::type_identifier(context.get_char_id());
    auto char_type    = [&](){ return fed::ast::type(auto(char_ident)); };
    auto char_type_handle = [&](){ return ast::handle(ast::type(auto(char_ident))); };
    auto parser      = fed::parser(
        ""_fv,
        std::move(context), 
        diagnostics
    );

    auto make_group_of_handles = []<typename... Ts>(Ts... handles) {
        auto group = ast::group<first<Ts...>>();
        (group.push_back(std::move(handles)), ...);
        return group;
    };
#   define ADD_TEST(case_info) do {\
        auto parse_case = parse_type_case case_info;\
        parser.remount(parse_case.source);\
        auto parse_res = parser.parse_type();\
        if (not parse_res.has_value()) {\
            FAIL(parse_res.error().message());\
        }\
        REQUIRE(*parse_res == parse_case.expected_result);\
    } while (false)


    SECTION("Builtin types") {
        ADD_TEST(({
            .source = "Integer"_fv, 
            .expected_result = int_type(),
        }));
        ADD_TEST(({
            .source = "Real"_fv, 
            .expected_result = real_type(),
        }));
        ADD_TEST(({
            .source = "Boolean"_fv, 
            .expected_result = bool_type(),
        }));
        ADD_TEST(({
            .source = "Char"_fv, 
            .expected_result = char_type(),
        }));
    }
    SECTION("Pointer types") {
        ADD_TEST(({
            .source = "^Integer"_fv, 
            .expected_result = fed::ast::pointer_type{
                .base = int_type(),
            }
        }));
        ADD_TEST(({
            .source = "^^^Real"_fv, 
            .expected_result = fed::ast::pointer_type{
                .base = fed::ast::pointer_type{
                    .base = fed::ast::pointer_type{
                        .base = real_type(),
                    }
                }
            }
        }));
    }
    SECTION("Array types") {
        using group_of_types = ast::group<ast::handle<ast::type>>;
        ADD_TEST(({
            .source = "array [Integer] of Real"_fv, 
            .expected_result = ast::array_type{
                .index_types = make_group_of_handles(int_type_handle()),
                .component_type = real_type()
            }
        }));
        ADD_TEST(({
            .source = "array [Integer, Char, Boolean] of Real"_fv, 
            .expected_result = ast::array_type{
                .index_types = make_group_of_handles(
                    int_type_handle(), char_type_handle(), bool_type_handle()
                ),
                .component_type = real_type()
            }
        }));
        ADD_TEST(({
            .source = "array [Integer] of array [Char] of Real"_fv, 
            .expected_result = ast::array_type{
                .index_types = make_group_of_handles(int_type_handle()),
                .component_type = ast::array_type{
                    .index_types = make_group_of_handles(char_type_handle()),
                    .component_type = real_type(),
                },
            }
        }));
        ADD_TEST(({
            .source = "array [Integer, Char] of array [Char, Integer] of Real"_fv, 
            .expected_result = ast::array_type{
                .index_types = make_group_of_handles(
                    int_type_handle(), char_type_handle()
                ),
                .component_type = ast::array_type{
                    .index_types = make_group_of_handles(
                        char_type_handle(), int_type_handle()
                    ),
                    .component_type = real_type(),
                },
            }
        }));
    }
    SECTION("Function types") {
        parser.remount("function () : Integer"_fv);
        parser.remount("function (:Integer) :Integer"_fv);
        parser.remount("function (:Integer, :Char) :Integer"_fv);
        parser.remount("function (:function (:Integer) :Integer) :Integer"_fv);
        parser.remount("function () :function (:Integer) :Real"_fv);
    }
}

template<>
struct Catch::StringMaker<fed::ast::type> {
    static auto convert(fed::ast::type const& val)
        -> std::string { return fmt::format("{}", val); }

};


