#include "fed/scanner/lex.hpp"
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

    result = token_view(
        m_source.subview(start, m_cursor),
        token_type::identifier
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
        // for now handles only small letters
        case 'a':case 'b':case 'c':case 'd':case 'e':case 'f':
        case 'g':case 'h':case 'j':case 'k':case 'l':case 'm':
        case 'n':case 'o':case 'p':case 'q':case 'r':case 's':
        case 't':case 'u':case 'v':case 'w':case 'x':case 'y':
        case 'z': {
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



