#include "fed/scanner/lex.hpp"
#include "fed/scanner/token.hpp"
#include "fed/diagnostics/buffer.hpp"

#include <string_view>
#include <array>

#include <catch2/catch_test_macros.hpp>




using namespace std::literals;

namespace Catch {
    template<>
    struct StringMaker<fed::token_type> {
        static std::string convert(fed::token_type const& value) {
            return fed::to_string(value);
        }
    };
}

TEST_CASE("Empty check", "[frontend][scanner]") {
    auto diagnostics = fed::diagnostics_buffer();
    SECTION("Empty source") {
        auto lex = fed::lexer(diagnostics, ""sv);
        REQUIRE(lex.lex_next_token().type() == fed::token_type::eof);
    }
    SECTION("Just spaces") {
        auto lex = fed::lexer(diagnostics, "             "sv);
        REQUIRE(lex.lex_next_token().type() == fed::token_type::eof);
    }
    SECTION("Just tabs") {
        auto lex = fed::lexer(diagnostics, "\t\t\t\t\t\t\t\t\t"sv);
        REQUIRE(lex.lex_next_token().type() == fed::token_type::eof);
    }
    SECTION("Just newlines") {
        auto lex = fed::lexer(diagnostics, "\n\n\n\n\n\n\n\n\n"sv);
        REQUIRE(lex.lex_next_token().type() == fed::token_type::eof);
    }
    SECTION("Mix") {
        auto lex = fed::lexer(diagnostics, "   \t\t  \n\n \t\n\t\t \n\n "sv);
        REQUIRE(lex.lex_next_token().type() == fed::token_type::eof);
    }

    // TODO: add tests for comments
}

TEST_CASE("Individual tokens", "[frontend][scanner]") {

    auto diagnostics = fed::diagnostics_buffer();
    SECTION("Special symbols") {
#       define TEST_TOKEN(token_str, token_t) do {\
            SECTION("'"#token_str"' token") {\
                auto lex = fed::lexer(diagnostics, #token_str ## sv);\
                auto const token = lex.lex_next_token();\
                CHECK(token.type() == fed::token_type::token_t);\
                CHECK(token.view().base() == #token_str ## sv);\
            }\
        } while (false)

        TEST_TOKEN(+, plus);
        TEST_TOKEN(-, minus);
        TEST_TOKEN(>, greater_than);
        TEST_TOKEN(<, less_than);
        TEST_TOKEN(>=, greater_or_equal_than);
        TEST_TOKEN(<=, less_or_equal_than);
        TEST_TOKEN(<>, not_equal);
        TEST_TOKEN(=, equal);
        TEST_TOKEN(., dot);
        TEST_TOKEN(.., dotdot);
        TEST_TOKEN(^, caret);
        TEST_TOKEN(/, slash);
        TEST_TOKEN(*, star);
        TEST_TOKEN(;, semicolon);
        TEST_TOKEN([, l_square);
        TEST_TOKEN(], r_square);
        TEST_TOKEN(:, colon);
        TEST_TOKEN(:=, define);

#       undef TEST_TOKEN
        // these are manualy written because of the limitations of the preprocessor
        SECTION("',' token") {
            auto lex = fed::lexer(diagnostics, ","sv);
            auto const token = lex.lex_next_token();
            CHECK(token.type() == fed::token_type::comma);
            CHECK(token.view().base() == ","sv);
        }
        SECTION("'(' token") {
            auto lex = fed::lexer(diagnostics, "("sv);
            auto const token = lex.lex_next_token();
            CHECK(token.type() == fed::token_type::l_paren);
            CHECK(token.view().base() == "("sv);
        }
        SECTION("')' token") {
            auto lex = fed::lexer(diagnostics, ")"sv);
            auto const token = lex.lex_next_token();
            CHECK(token.type() == fed::token_type::r_paren);
            CHECK(token.view().base() == ")"sv);
        }
    }

    SECTION("Keywords") {
#       define TEST_KEYWORD(keyword_token) do {\
            SECTION("'"#keyword_token"' keyword") {\
                auto lex = fed::lexer(diagnostics, #keyword_token ## sv);\
                auto const token = lex.lex_next_token();\
                CHECK(token.type() == fed::token_type::keyword_ ## keyword_token);\
                CHECK(token.view().base() == #keyword_token ## sv);\
            }\
        } while (false)
        
        TEST_KEYWORD(and);
        TEST_KEYWORD(array);
        TEST_KEYWORD(begin);
        TEST_KEYWORD(case);
        TEST_KEYWORD(const);
        TEST_KEYWORD(div);
        TEST_KEYWORD(do);
        TEST_KEYWORD(downto);
        TEST_KEYWORD(else);
        TEST_KEYWORD(end);
        TEST_KEYWORD(file);
        TEST_KEYWORD(for);
        TEST_KEYWORD(function);
        TEST_KEYWORD(goto);
        TEST_KEYWORD(in);
        TEST_KEYWORD(if);
        TEST_KEYWORD(label);
        TEST_KEYWORD(mod);
        TEST_KEYWORD(nil);
        TEST_KEYWORD(not);
        TEST_KEYWORD(or);
        TEST_KEYWORD(of);
        TEST_KEYWORD(packed);
        TEST_KEYWORD(procedure);
        TEST_KEYWORD(program);
        TEST_KEYWORD(record);
        TEST_KEYWORD(repeat);
        TEST_KEYWORD(set);
        TEST_KEYWORD(then);
        TEST_KEYWORD(to);
        TEST_KEYWORD(type);
        TEST_KEYWORD(until);
        TEST_KEYWORD(var);
        TEST_KEYWORD(while);
        TEST_KEYWORD(with);


#       undef TEST_KEYWORD
    }

    SECTION("literal token") {
        auto lex = fed::lexer(diagnostics, "'mama ama literal'"sv);
        auto const token = lex.lex_next_token();
        CHECK(token.type() == fed::token_type::literal);
        CHECK(token.view().base() == "'mama ama literal'"sv);
    }

}

TEST_CASE("Individual identifiers", "[frontend][scanner]") {
    auto diagnostics = fed::diagnostics_buffer();
    SECTION("Identifiers can contain lower case letters") {
        auto lexer = fed::lexer(diagnostics, "abcdefg"sv);
        auto const token = lexer.lex_next_token();
        CHECK(token.type() == fed::token_type::identifier);
        CHECK(token.view().base() == "abcdefg"sv);
    }
    SECTION("Identifiers can contain upper case letters") {
        auto lexer = fed::lexer(diagnostics, "ABCDEFG"sv);
        auto const token = lexer.lex_next_token();
        CHECK(token.type() == fed::token_type::identifier);
        CHECK(token.view().base() == "ABCDEFG"sv);
    }
    SECTION("Identifiers can contain one letter and numbers") {
        auto lexer = fed::lexer(diagnostics, "a12345"sv);
        auto const token = lexer.lex_next_token();
        CHECK(token.type() == fed::token_type::identifier);
        CHECK(token.view().base() == "a12345"sv);
    }
    SECTION("Identifiers can contain keywords inside") {
        auto lexer = fed::lexer(diagnostics, "formaldehyteroquefortfeoffor"sv);
        auto const token = lexer.lex_next_token();
        CHECK(token.type() == fed::token_type::identifier);
        CHECK(token.view().base() == "formaldehyteroquefortfeoffor"sv);
    }
    SECTION("Identifiers can contain a little bit of everything") {
        auto lexer = fed::lexer(diagnostics, "IAmProudOwnerOf7Kittens"sv);
        auto const token = lexer.lex_next_token();
        CHECK(token.type() == fed::token_type::identifier);
        CHECK(token.view().base() == "IAmProudOwnerOf7Kittens"sv);
    }
}

TEST_CASE("Lexing numbers", "[frontend][scanner]") {
    auto diagnostics = fed::diagnostics_buffer();
    SECTION("Integer numbers") {
        auto lexer = fed::lexer(diagnostics, "1234567890"sv);
        auto const token = lexer.lex_next_token();
        CHECK(token.type() == fed::token_type::number_integer);
        CHECK(token.view().base() == "1234567890"sv);
        if (diagnostics.current_error_count() != 0) {
            diagnostics.flush();
            FAIL("Failed to lex\n");
        }
    }
    SECTION("Real numbers") {
#define TEST_NUMBER(num) \
    do {\
        auto lexer = fed::lexer(diagnostics, #num ##sv);\
        auto const token = lexer.lex_next_token();\
        CHECK(token.type() == fed::token_type::number_real);\
        CHECK(token.view().base() == #num ##sv);\
    } while (false) 

        TEST_NUMBER(0.511);
        TEST_NUMBER(4e0);
        TEST_NUMBER(4e+1);
        TEST_NUMBER(4e-1);
        TEST_NUMBER(0.4e-1);
        TEST_NUMBER(0.4e+1);
#undef TEST_NUMBER
        if (diagnostics.current_error_count() != 0) {
            diagnostics.flush();
            FAIL("Failed to lex\n");
        }
    }
}

TEST_CASE("Lex multiple tokens", "[frontend][lexer]") {
    auto diagnostics = fed::diagnostics_buffer();
    SECTION("With spaces") {
        auto lexer = fed::lexer(diagnostics, "abc + def"sv);
        auto lhs_token = lexer.lex_next_token();
        lexer.advance_lexer();
        auto op_token  = lexer.lex_next_token();
        lexer.advance_lexer();
        auto rhs_token = lexer.lex_next_token();
        lexer.advance_lexer();
        CHECK(lhs_token.type() == fed::token_type::identifier);
        CHECK(op_token.type()  == fed::token_type::plus);
        CHECK(rhs_token.type() == fed::token_type::identifier);
        CHECK(lhs_token.view().base() == "abc"sv);
        CHECK(rhs_token.view().base() == "def"sv);
        CHECK(op_token.view().base()  == "+"sv);
    }
    SECTION("Without spaces") {
        auto lexer = fed::lexer(diagnostics, "abc+def"sv);
        auto lhs_token = lexer.lex_next_token();
        lexer.advance_lexer();
        auto op_token  = lexer.lex_next_token();
        lexer.advance_lexer();
        auto rhs_token = lexer.lex_next_token();
        lexer.advance_lexer();
        CHECK(lhs_token.type() == fed::token_type::identifier);
        CHECK(op_token.type()  == fed::token_type::plus);
        CHECK(rhs_token.type() == fed::token_type::identifier);
        CHECK(lhs_token.view().base() == "abc"sv);
        CHECK(rhs_token.view().base() == "def"sv);
        CHECK(op_token.view().base()  == "+"sv);
    }
    if (diagnostics.current_error_count() != 0) {
        diagnostics.flush();
        FAIL("Failed to lex\n");
    }
}

TEST_CASE("Lex small program", "[frontend][lexer]") {
    auto const program = 
        "program hello(output);\n"
        "\n"
        "type\n"
        "   CharSet = set of Char;\n"
        "var\n"
        "   C : Char;\n"
        "   CS1, CS2 : CharSet;\n"
        "begin\n"
        "CS1 := ['A', 'B', 'C', 'D'];\n"
        "CS2 := ['E', 'F', 'G'];\n"
        "if not (C in CS1 * CS2) then writeln('false');\n"
        "end.\n"sv;
    using enum fed::token_type;
    auto const expected = std::array{
        keyword_program, identifier, l_paren, identifier, r_paren, semicolon,
        keyword_type,
        identifier, equal, keyword_set, keyword_of, identifier, semicolon,
        keyword_var,
        identifier, colon, identifier, semicolon,
        identifier, comma, identifier, colon, identifier, semicolon,
        keyword_begin,
        identifier, define, l_square, 
            literal, comma, literal, comma,
            literal, comma, literal,
        r_square, semicolon,
        identifier, define, l_square,
            literal, comma, literal, comma, literal,
        r_square, semicolon,
        keyword_if, keyword_not, l_paren, 
            identifier, keyword_in, identifier,
            star, identifier,
        r_paren, keyword_then, identifier, l_paren,
            literal,
        r_paren, semicolon,
        keyword_end, dot, eof,
    };
    auto diagnostics = fed::diagnostics_buffer();
    auto lexer       = fed::lexer(diagnostics, program);
    auto const result = [&]() {
        auto result = std::vector<fed::token_type>();
        auto last_token = fed::token_type::eof;
        do {
            last_token = lexer.lex_next_token().type();
            result.push_back(last_token);
            lexer.advance_lexer();
        } while (last_token != fed::token_type::eof);
        return result;
    }();
    auto const common_size = std::min(expected.size(), result.size());

    for (std::size_t i = 0; i < common_size; ++i) {
        CHECK(expected[i] == result[i]);
    }
}
