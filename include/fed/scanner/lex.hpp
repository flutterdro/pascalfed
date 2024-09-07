#ifndef FED_LEX_HPP_
#define FED_LEX_HPP_

#include "token.hpp"
#include "fed/representations/raw-source.hpp"

#include <string_view>

namespace fed {

struct lexer {

    lexer(source::full_view view);

    lexer(lexer const&) = default;
    lexer(lexer&&) noexcept = default;
    auto operator=(lexer const&)
        -> lexer& = default;
    auto operator=(lexer&&)
        -> lexer& = default;

    auto cursor() const noexcept
        -> source::iterator;

    [[nodiscard]]auto lex_next_token() noexcept
        -> token_view;
    auto advance_lexer()
        -> void;
    
    template<typename F>
    auto current_char_is(F&& predicate)
        -> bool { return m_cursor != m_source.end() and predicate(*m_cursor); }
    auto current_char_is(char character) 
        -> bool { return m_cursor != m_source.end() and *m_cursor == character; }

    auto lex_as_word() noexcept
        -> token_view;
    auto lex_as_literal() noexcept
        -> token_view;
    auto lex_as_number() noexcept
        -> token_view;
    
    source::full_view m_source;
    source::iterator m_cursor;
    token_view m_cached_token;
    bool m_is_relexing;
};


} // namespace fed


#endif
