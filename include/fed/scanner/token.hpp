#ifndef FED_TOKEN_HPP_
#define FED_TOKEN_HPP_

#include <string_view>

namespace fed {

enum class token_type {
    keyword_and,
    keyword_array,
    keyword_begin,
    keyword_case,
    keyword_const,
    keyword_div,
    keyword_do,
    keyword_downto,
    keuword_else,
    keyword_end,
    keyword_file,
    keyword_for,
    keyword_function,
    keyword_goto,
    keyword_if,
    keyword_in,
    keyword_label,
    keyword_mod,
    keyword_nil,
    keyword_not,
    keyword_of,
    keyword_or,
    keyword_packed,
    keyword_procedure,
    keyword_program,
    keyword_record,
    keyword_repeat,
    keyword_set,
    keyword_then,
    keyword_to,
    keyword_type,
    keyword_until,
    keyword_var,
    keyword_while,
    keyword_with,
    plus,
    minus,
    star,
    slash,
    equal,
    less_than,
    greater_than,
    l_square,
    r_square,
    dot,
    comma,
    colon,
    semicolon,
    caret,
    l_paren,
    r_paren,
    not_equal,
    less_or_equal_than,
    greater_or_equal_than,
    define,
    dotdot,
    identifier,
    number,
    literal,
    eof,
    not_a_token,
};

struct token_view {
    [[nodiscard]]constexpr auto type() const noexcept
        -> token_type const& { return m_type; }
    [[nodiscard]]constexpr auto view() const noexcept
        -> std::string_view const& { return m_view; }
    std::string_view m_view;
    token_type m_type;
};

} // namespace fed 


#endif
