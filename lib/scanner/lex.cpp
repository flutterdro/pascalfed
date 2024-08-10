#include "fed/scanner/lex.hpp"
#include "fed/scanner/token.hpp"
#include "fed/utils/superutil.hpp"


#include <string_view>
#include <algorithm>


namespace fed {

using namespace std::literals;
namespace stdr = std::ranges;



auto lexer::advance_lexer()
    -> void {
    // TODO: add error handling
    // probably throw an exception
    if (m_cached_token.m_type == token_type::eof) { return; }
    if (not m_is_relexing) {}

    m_is_relexing = false;
}

constexpr auto lex_as_word(std::string_view source) noexcept 
    -> token_view {
    auto result = token_view{};
    auto token_size = std::string_view::size_type{1};

    auto is_valid_identifier_symbol = func::any_of(
            "abcdefghijklmnopqrstuvwxyz"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "1234567890"sv
        );

    while (token_size < source.size() and 
        is_valid_identifier_symbol(source[token_size])) {
        ++token_size;
    }

    result = token_view{
        .m_view = source.substr(0, token_size),
        .m_type = token_type::identifier,
    };

    return result;
}

constexpr auto lex_as_literal(std::string_view source) 
    -> token_view {
    auto result = token_view{};
    auto token_size = std::size_t{1};
    
    for (std::size_t i = 1; source[i] != '\''; ++i, ++token_size) {}

    result = token_view{
        .m_view = source.substr(0, token_size),
        .m_type = token_type::literal,
    };

    return result;
}

constexpr auto lex_as_number(std::string_view source)
    -> token_view {
    auto result = token_view{};

    return result;
}

auto lexer::lex_next_token() noexcept 
    -> token_view {
    auto result = token_view{};
    if (m_is_relexing) {
        result = m_cached_token;
        return result;
    }
    m_cursor = stdr::find_if(m_cursor, m_source.end(), func::no(func::any_of("\t \n"sv)));
    switch(m_source[0]) {
        // for now handles only small letters
        case 'a':case 'b':case 'c':case 'd':case 'e':case 'f':
        case 'g':case 'h':case 'j':case 'k':case 'l':case 'm':
        case 'n':case 'o':case 'p':case 'q':case 'r':case 's':
        case 't':case 'u':case 'v':case 'w':case 'x':case 'y':
        case 'z': {
            result = fed::lex_as_word(m_source);
            break;
        }
        case '0':case '1':case '2':case '3':case '4':case '5':
        case '6':case '7':case '8':case '9': {
            result = fed::lex_as_number(m_source);
            break;
        }
        case '\'': {
            result = fed::lex_as_literal(m_source);
            break;
        }
        case '+': {
            result = token_view{
                .m_view = m_source.substr(0, 1),
                .m_type = token_type::plus,
            };
            break;
        }
        case '-': {
            result = token_view{
                .m_view = m_source.substr(0, 1),
                .m_type = token_type::minus,
            };
            break;
        }
        case '*': {
            result = token_view{
                .m_view = m_source.substr(0, 1),
                .m_type = token_type::star,
            };
            break;
        }
        case '/': {
            result = token_view{
                .m_view = m_source.substr(0, 1),
                .m_type = token_type::slash,
            };
            break;
        }
        case ';': {
            result = token_view{
                .m_view = m_source.substr(0, 1),
                .m_type = token_type::semicolon,
            };
            break;
        }
        case '^': {
            result = token_view{
                .m_view = m_source.substr(0, 1),
                .m_type = token_type::caret,
            };
            break;
        }
        case '[': {
            result = token_view{
                .m_view = m_source.substr(0, 1),
                .m_type = token_type::l_square,
            };
            break;
        }
        case ']': {
            result = token_view{
                .m_view = m_source.substr(0, 1),
                .m_type = token_type::r_square,
            };
            break;
        }
        case ')': {
            result = token_view{
                .m_view = m_source.substr(0, 1),
                .m_type = token_type::r_paren,
            };
            break;
        }
        case '(': {
            result = token_view{
                .m_view = m_source.substr(0, 1),
                .m_type = token_type::l_paren,
            };
            break;
        }
        case ',': {
            result = token_view{
                .m_view = m_source.substr(0, 1),
                .m_type = token_type::comma,
            };
            break;
        }
        case '=': {
            result = token_view{
                .m_view = m_source.substr(0, 1),
                .m_type = token_type::equal,
            };
            break;
        }
        case '<': {
            if (m_source.size() > 1) {
                if (m_source[1] == '>') {
                    result = token_view{
                        .m_view = m_source.substr(0, 2),
                        .m_type = token_type::not_equal,
                    };
                    break;
                } else if (m_source[1] == '=') {
                    result = token_view{
                        .m_view = m_source.substr(0, 2),
                        .m_type = token_type::less_or_equal_than,
                    };
                    break;
                }
            }
            result = token_view{
                .m_view = m_source.substr(0, 1),
                .m_type = token_type::less_than,
            };
            break;
        }
        case '>': {
            if (m_source.size() > 1 and m_source[1] == '=') {
                result = token_view{
                    .m_view = m_source.substr(0, 2),
                    .m_type = token_type::greater_or_equal_than,
                };
            } else {
                result = token_view{
                    .m_view = m_source.substr(0, 1),
                    .m_type = token_type::greater_than,
                };
            }
            break;
        }
        case '.': {
            if (m_source.size() > 1 and m_source[1] == '.') {
                result = token_view{
                    .m_view = m_source.substr(0, 2),
                    .m_type = token_type::dotdot,
                };
            } else {
                result = token_view{
                    .m_view = m_source.substr(0, 1),
                    .m_type = token_type::dot,
                };
            }
            break;
        }
        default: {
            result = token_view{
                .m_view = {},
                .m_type = token_type::not_a_token,
            };
            break;
        }
    }

    m_is_relexing = true;
    m_cached_token = result;

    return result;
}


} // namespace fed



