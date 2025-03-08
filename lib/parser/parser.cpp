#include "fed/parser/parser.hpp"
#include "fed/diagnostics/buffer.hpp"
#include "fed/scanner/token.hpp"


namespace fed {

parser::parser(source::full_view source, diagnostics_buffer& buffer)
    : m_lexer(buffer, source), m_diagnostics(buffer) {}

auto parser::cursor() const noexcept
    -> source::iterator {
    return m_lexer.cursor();
}

auto parser::maybe_consume_and_advance_expecting(token_type token)
    -> bool {
    auto tok = consume_and_advance();
    return tok.type() == token;
}
auto parser::current_token()
    -> token_view { return m_lexer.lex_next_token(); }
auto parser::consume_and_advance()
    -> token_view { 
    auto token = m_lexer.lex_next_token();
    m_lexer.advance_lexer();
    return token;
}
 


} // namespace fed
