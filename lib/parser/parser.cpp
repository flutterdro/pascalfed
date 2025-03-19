#include "fed/parser/parser.hpp"
#include "fed/diagnostics/buffer.hpp"
#include "fed/diagnostics/compile-error.hpp"
#include "fed/parser/context.hpp"
#include "fed/scanner/token.hpp"
#include <variant>


namespace fed {

parser::parser(
    source::full_view source, 
    semantic_context context,
    diagnostics_buffer& buffer
)
    : m_lexer(buffer, source), m_diagnostics(buffer), m_context(std::move(context)) {}

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
auto parser::diagnostics() noexcept
    -> diagnostics_buffer& { return m_diagnostics; }
auto parser::push_error(compilation_error err) 
    -> std::monostate { 
    diagnostics().push_back(std::move(err)); 
    return std::monostate();
}
auto parser::context() noexcept
    -> semantic_context& { return m_context; }
auto parser::consume_and_advance_expecting(token_type token)
    -> void {
    if (current_token().type() == token) {
        consume_and_advance();
    }
    else  push_error(parse_error());

}

} // namespace fed
