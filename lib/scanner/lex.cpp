#include "fed/scanner/lex.hpp"
#include "fed/representations/parse-tree.hpp"
#include "fed/representations/raw-source.hpp"
#include "fed/scanner/token.hpp"
#include "fed/utils/superutil.hpp"
#include "fed/diagnostics/internal-error.hpp"


#include <string_view>
#include <algorithm>


namespace fed {

using namespace std::literals;
namespace stdr = std::ranges;


lexer::lexer(source::full_view view) 
    :
        m_source(view), 
        m_cursor(m_source.begin()),
        m_cached_token(),
        m_is_relexing(false)
{}


auto lexer::advance_lexer()
    -> void {
    // TODO: add error handling
    // probably throw an exception
    if (m_cached_token.m_type == token_type::eof) { return; }
    if (not m_is_relexing) {}

    m_is_relexing = false;
}

auto check_for_keyword(std::string_view identifier) 
    -> token_type {

    // TODO: compare hashes, not strings
    if (identifier == "and"sv) return token_type::keyword_and;
    if (identifier == "array"sv) return token_type::keyword_array;
    if (identifier == "begin"sv) return token_type::keyword_begin;
    if (identifier == "case"sv) return token_type::keyword_case;
    if (identifier == "const"sv) return token_type::keyword_const;
    if (identifier == "div"sv) return token_type::keyword_div;
    if (identifier == "do"sv) return token_type::keyword_do;
    if (identifier == "downto"sv) return token_type::keyword_downto;
    if (identifier == "else"sv) return token_type::keyword_else;
    if (identifier == "end"sv) return token_type::keyword_end;
    if (identifier == "file"sv) return token_type::keyword_file;
    if (identifier == "for"sv) return token_type::keyword_for;
    if (identifier == "function"sv) return token_type::keyword_function;
    if (identifier == "goto"sv) return token_type::keyword_goto;
    if (identifier == "if"sv) return token_type::keyword_if;
    if (identifier == "in"sv) return token_type::keyword_in;
    if (identifier == "label"sv) return token_type::keyword_label;
    if (identifier == "mod"sv) return token_type::keyword_mod;
    if (identifier == "nil"sv) return token_type::keyword_nil;
    if (identifier == "not"sv) return token_type::keyword_not;
    if (identifier == "of"sv) return token_type::keyword_of;
    if (identifier == "or"sv) return token_type::keyword_or;
    if (identifier == "packed"sv) return token_type::keyword_packed;
    if (identifier == "procedure"sv) return token_type::keyword_procedure;
    if (identifier == "program"sv) return token_type::keyword_program;
    if (identifier == "record"sv) return token_type::keyword_record;
    if (identifier == "repeat"sv) return token_type::keyword_repeat;
    if (identifier == "set"sv) return token_type::keyword_set;
    if (identifier == "then"sv) return token_type::keyword_then;
    if (identifier == "to"sv) return token_type::keyword_to;
    if (identifier == "type"sv) return token_type::keyword_type;
    if (identifier == "until"sv) return token_type::keyword_until;
    if (identifier == "var"sv) return token_type::keyword_var;
    if (identifier == "while"sv) return token_type::keyword_while;
    if (identifier == "with"sv) return token_type::keyword_with;


    return token_type::identifier;
}
auto lexer::lex_as_word() noexcept 
    -> token_view {
    auto result = token_view();
    auto const start = m_cursor;

    auto is_valid_identifier_symbol = func::any_of(
            "abcdefghijklmnopqrstuvwxyz"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "1234567890"sv
        );

    while (m_cursor != m_source.end() and 
        is_valid_identifier_symbol(*m_cursor++)) {}

    auto const identifier_view = source::view(start, m_cursor);


    result = token_view(
        identifier_view,
        check_for_keyword(identifier_view.base())
    );

    return result;
}

auto lexer::lex_as_literal() noexcept
    -> token_view {
    auto result = token_view();
    auto const start = m_cursor++;
    
    while (m_cursor != m_source.end() and *m_cursor++ != '\'') {}

    result = token_view(
        m_source.subview(start, m_cursor),
        token_type::literal
    );

    return result;
}

auto lexer::lex_as_number() noexcept
    -> token_view {
    auto result = token_view();
    auto result_token_type = token_type::number_integer;
    auto const start = m_cursor;

    auto is_digit = [](char character) { return std::isdigit(character); };

    while (++m_cursor != m_source.end() and std::isdigit(*m_cursor)) {}

    if (m_cursor != m_source.end()) {
        if (*m_cursor == '.') {
            result_token_type = token_type::number_real;
            while (++m_cursor != m_source.end() and std::isdigit(*m_cursor)) {}
        }
        if (current_char_is('e')) {
            ++m_cursor;
            result_token_type = token_type::number_real;
            if (current_char_is('+') or current_char_is('-')) {
                ++m_cursor;
            }
            while (current_char_is(is_digit)) { ++m_cursor; }
        }

    }

    return result;
}

auto lexer::lex_next_token() noexcept 
    -> token_view {
    auto result = token_view();
    
    // if we call this without prior advance_lexer() 
    // then we can just return cached result
    if (m_is_relexing) {
        result = m_cached_token;
        return result;
    }
    
    // skip whitespaces etc
    m_cursor = stdr::find_if(
        m_cursor, m_source.end(), 
        func::no(func::any_of("\t \n"sv))
    );

    if (m_cursor == m_source.end()) {
        result = token_view(
            source::view(),
            token_type::eof
        );
        return result;
    }

    switch(*m_cursor) {
        case 'A':case 'B':case 'C':case 'D':case 'E':case 'F':
        case 'G':case 'H':case 'I':case 'J':case 'K':case 'L':
        case 'M':case 'N':case 'O':case 'P':case 'Q':case 'R':
        case 'S':case 'T':case 'U':case 'V':case 'W':case 'X':
        case 'Y':case 'Z':case 'a':case 'b':case 'c':case 'd':
        case 'e':case 'f':case 'g':case 'h':case 'i':case 'j':
        case 'k':case 'l':case 'm':case 'n':case 'o':case 'p':
        case 'q':case 'r':case 's':case 't':case 'u':case 'v':
        case 'w':case 'x':case 'y':case 'z': {
            result = lex_as_word();
            break;
        }
        case '0':case '1':case '2':case '3':case '4':case '5':
        case '6':case '7':case '8':case '9': {
            result = lex_as_number();
            break;
        }
        case '\'': {
            result = lex_as_literal();
            break;
        }
        case '+': {
            auto const start = m_cursor++;
            result = token_view(
                m_source.subview(start, m_cursor),
                token_type::plus
        );
            break;
        }
        case '-': {
            auto const start = m_cursor++;
            result = token_view(
                m_source.subview(start, m_cursor),
                token_type::minus
            );
            break;
        }
        case '*': {
            auto const start = m_cursor++;
            result = token_view(
                m_source.subview(start, m_cursor),
                token_type::star
            );
            break;
        }
        case '/': {
            auto const start = m_cursor++;
            result = token_view(
                m_source.subview(start, m_cursor),
                token_type::slash
            );
            break;
        }
        case ';': {
            auto const start = m_cursor++;
            result = token_view(
                m_source.subview(start, m_cursor),
                token_type::semicolon
            );
            break;
        }
        case '^': {
            auto const start = m_cursor++;
            result = token_view(
                m_source.subview(start, m_cursor),
                token_type::caret
            );
            break;
        }
        case '[': {
            auto const start = m_cursor++;
            result = token_view(
                m_source.subview(start, m_cursor),
                token_type::l_square
            );
            break;
        }
        case ']': {
            auto const start = m_cursor++;
            result = token_view(
                m_source.subview(start, m_cursor),
                token_type::r_square
            );
            break;
        }
        case ')': {
            auto const start = m_cursor++;
            result = token_view(
                m_source.subview(start, m_cursor),
                token_type::r_paren
            );
            break;
        }
        case '(': {
            auto const start = m_cursor++;
            result = token_view(
                m_source.subview(start, m_cursor),
                token_type::l_paren
            );
            break;
        }
        case ',': {
            auto const start = m_cursor++;
            result = token_view(
                m_source.subview(start, m_cursor),
                token_type::comma
            );
            break;
        }
        case '=': {
            auto const start = m_cursor++;
            result = token_view(
                m_source.subview(start, m_cursor),
                token_type::equal
            );
            break;
        }
        case '<': {
            auto const start = m_cursor++;
            if (m_cursor != m_source.end()) {
                if (*m_cursor == '>') {
                    result = token_view(
                        m_source.subview(start, ++m_cursor),
                        token_type::not_equal
                    );
                    break;
                } else if (*m_cursor == '=') {
                    result = token_view(
                        m_source.subview(start, ++m_cursor),
                        token_type::less_or_equal_than
                    );
                    break;
                }
            }
            result = token_view(
                m_source.subview(start, m_cursor),
                token_type::less_than
            );
            break;
        }
        case '>': {
            auto const start = m_cursor++;
            if (m_cursor != m_source.end() and *m_cursor == '=') {
                result = token_view(
                    m_source.subview(start, ++m_cursor),
                    token_type::greater_or_equal_than
                );
            } else {
                result = token_view(
                    m_source.subview(start, m_cursor),
                    token_type::greater_than
                );
            }
            break;
        }
        case '.': {
            auto const start = m_cursor++;
            if (m_cursor != m_source.end() and *m_cursor == '.') {
                result = token_view(
                    m_source.subview(start, ++m_cursor),
                    token_type::dotdot
                );
            } else {
                result = token_view(
                    m_source.subview(start, m_cursor),
                    token_type::dot
                );
            }
            break;
        }
        case ':': {
            auto const start = m_cursor++;
            if (m_cursor != m_source.end() and *m_cursor == '=') {
                result = token_view(
                    source::view(start, ++m_cursor),
                    token_type::define
                );
            } else {
                result = token_view(
                    source::view(start, m_cursor),
                    token_type::colon
                );
            }
            break;
        }
        default: {
            result = token_view(
                source::view(),
                token_type::not_a_token
            );
            break;
        }
    }

    m_is_relexing = true;
    m_cached_token = result;

    return result;
}


} // namespace fed



