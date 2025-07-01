#include "fed/parser/parser.hpp"
#include "fed/diagnostics/buffer.hpp"
#include "fed/diagnostics/compile-error.hpp"
#include "fed/parser/context.hpp"
#include "fed/representations/raw-source.hpp"
#include "fed/scanner/token.hpp"
#include <variant>


namespace fed {

parser::parser(
    source::full_view source, 
    diagnostics_buffer& buffer
)
    : m_lexer(buffer, source)
    , m_diagnostics(buffer) 
    , m_hazard_terminators() {
    m_hazard_terminators.reserve(10);
    m_hazard_terminators.push_back(token_type::eof);
}

auto parser::remount(source::full_view view)
    -> void {
    m_lexer.remount(view);
}

auto parser::cursor() const noexcept
    -> source::iterator {
    return m_lexer.cursor();
}

auto parser::maybe_consume_and_advance_expecting(token_type token)
    -> bool {
    if (current_token_is(equal_to(token))) {
        consume_and_advance();
        return true;
    }
    return false;
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
auto parser::consume_and_advance_expecting(token_type token)
    -> parse_result<void> {
    if (current_token().type() == token) {
        consume_and_advance();
        return {};
    }
    else {
        return std::unexpected(missing_token(this->cursor().where(), token));
    }

}

auto parser::push_hazard_terminator(token_type token) 
    -> void {
    m_hazard_terminators.push_back(token);
}
auto parser::pop_hazard_terminator()
    -> void {
    if (m_hazard_terminators.size() <= 1) 
        throw internal_error("attempt to excessively pop terminator stack");
    m_hazard_terminators.pop_back();
}
auto parser::hazard_terminators() const noexcept
    -> token_stack const& {
    return m_hazard_terminators;
}

} // namespace fed
