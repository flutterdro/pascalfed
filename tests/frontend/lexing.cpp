#include "fed/scanner/lex.hpp"
#include "fed/scanner/token.hpp"

#include <string_view>

#include <catch2/catch_test_macros.hpp>




using namespace std::literals;


TEST_CASE("Empty check", "[frontend][scanner]") {
    SECTION("Empty source") {
        fed::lexer lex(""sv);
        REQUIRE(lex.lex_next_token().type() == fed::token_type::eof);
    }
    SECTION("Just spaces") {
        fed::lexer lex("             "sv);
        REQUIRE(lex.lex_next_token().type() == fed::token_type::eof);
    }
    SECTION("Just tabs") {
        fed::lexer lex("\t\t\t\t\t\t\t\t\t"sv);
        REQUIRE(lex.lex_next_token().type() == fed::token_type::eof);
    }
    SECTION("Just newlines") {
        fed::lexer lex("\n\n\n\n\n\n\n\n\n"sv);
        REQUIRE(lex.lex_next_token().type() == fed::token_type::eof);
    }
    SECTION("Mix") {
        fed::lexer lex("   \t\t  \n\n \t\n\t\t \n\n "sv);
        REQUIRE(lex.lex_next_token().type() == fed::token_type::eof);
    }

    // TODO: add tests for comments
}

TEST_CASE("Individual tokens", "[frontend][scanner]") {

    SECTION("Special symbols") {
#       define TEST_TOKEN(token_str, token_t) do {\
            SECTION("'"#token_str"' token") {\
                fed::lexer lex(#token_str ## sv);\
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
            fed::lexer lex(","sv);
            auto const token = lex.lex_next_token();
            CHECK(token.type() == fed::token_type::comma);
            CHECK(token.view().base() == ","sv);
        }
        SECTION("'(' token") {
            fed::lexer lex("("sv);
            auto const token = lex.lex_next_token();
            CHECK(token.type() == fed::token_type::l_paren);
            CHECK(token.view().base() == "("sv);
        }
        SECTION("')' token") {
            fed::lexer lex(")"sv);
            auto const token = lex.lex_next_token();
            CHECK(token.type() == fed::token_type::r_paren);
            CHECK(token.view().base() == ")"sv);
        }
    }

    SECTION("Keywords") {
#       define TEST_KEYWORD(keyword_token) do {\
            SECTION("'"#keyword_token"' keyword") {\
                fed::lexer lex(#keyword_token ## sv);\
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

}

