#ifndef FED_TOKEN_HPP_
#define FED_TOKEN_HPP_

#include "fed/representations/raw-source.hpp"
#include <string>
#include <utility>

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
    keyword_else,
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
    number_integer,
    number_real,
    literal,
    eof,
    empty,
    not_a_token,
};

constexpr auto to_string(token_type type) 
    -> std::string {
#define CASE_ENUM(member) \
    case token_type::member: return #member

    switch (type) {
        CASE_ENUM(keyword_and);
        CASE_ENUM(keyword_array);
        CASE_ENUM(keyword_begin);
        CASE_ENUM(keyword_case);
        CASE_ENUM(keyword_const);
        CASE_ENUM(keyword_div);
        CASE_ENUM(keyword_do);
        CASE_ENUM(keyword_downto);
        CASE_ENUM(keyword_else);
        CASE_ENUM(keyword_end);
        CASE_ENUM(keyword_file);
        CASE_ENUM(keyword_for);
        CASE_ENUM(keyword_function);
        CASE_ENUM(keyword_goto);
        CASE_ENUM(keyword_if);
        CASE_ENUM(keyword_in);
        CASE_ENUM(keyword_label);
        CASE_ENUM(keyword_mod);
        CASE_ENUM(keyword_nil);
        CASE_ENUM(keyword_not);
        CASE_ENUM(keyword_of);
        CASE_ENUM(keyword_or);
        CASE_ENUM(keyword_packed);
        CASE_ENUM(keyword_procedure);
        CASE_ENUM(keyword_program);
        CASE_ENUM(keyword_record);
        CASE_ENUM(keyword_repeat);
        CASE_ENUM(keyword_set);
        CASE_ENUM(keyword_then);
        CASE_ENUM(keyword_to);
        CASE_ENUM(keyword_type);
        CASE_ENUM(keyword_until);
        CASE_ENUM(keyword_var);
        CASE_ENUM(keyword_while);
        CASE_ENUM(keyword_with);
        CASE_ENUM(plus);
        CASE_ENUM(minus);
        CASE_ENUM(star);
        CASE_ENUM(slash);
        CASE_ENUM(equal);
        CASE_ENUM(less_than);
        CASE_ENUM(greater_than);
        CASE_ENUM(l_square);
        CASE_ENUM(r_square);
        CASE_ENUM(dot);
        CASE_ENUM(comma);
        CASE_ENUM(colon);
        CASE_ENUM(semicolon);
        CASE_ENUM(caret);
        CASE_ENUM(l_paren);
        CASE_ENUM(r_paren);
        CASE_ENUM(not_equal);
        CASE_ENUM(less_or_equal_than);
        CASE_ENUM(greater_or_equal_than);
        CASE_ENUM(define);
        CASE_ENUM(dotdot);
        CASE_ENUM(identifier);
        CASE_ENUM(number_integer);
        CASE_ENUM(number_real);
        CASE_ENUM(literal);
        CASE_ENUM(eof);
        CASE_ENUM(empty);
        CASE_ENUM(not_a_token);
        default: std::unreachable();
    }
}

struct token_view {
    [[nodiscard]]constexpr auto type() const noexcept
        -> token_type { return m_type; }
    [[nodiscard]]constexpr auto view() const noexcept
        -> source::view { return m_view; }
    source::view m_view;
    token_type m_type;
};

} // namespace fed 


#endif
