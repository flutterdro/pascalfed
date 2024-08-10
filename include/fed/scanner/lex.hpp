#ifndef FED_LEX_HPP_
#define FED_LEX_HPP_

#include "token.hpp"
#include "fed/representations/raw-source.hpp"

#include <string_view>

namespace fed {

struct lexer {
    [[nodiscard]]auto lex_next_token() noexcept
        -> token_view;
    auto advance_lexer()
        -> void;
    
    source::full_view m_source;
    source::full_view::iterator m_cursor;
    token_view m_cached_token;
    bool m_is_relexing;
};


} // namespace fed


#endif
