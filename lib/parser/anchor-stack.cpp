#include "fed/parser/anchor-stack.hpp"
#include "fed/scanner/token.hpp"
#include "fed/utils/macros.hpp"
#include "fed/diagnostics/internal-error.hpp"

#include <algorithm>
#include <fmt/format.h>
#include <fmt/ranges.h>

#include <ranges>


namespace fed {

anchor_stack::anchor_stack() {
    m_stack.reserve(12);
    m_stack.push_back(token_type::eof);
}

auto anchor_stack::top() const noexcept
    -> token_type { return m_stack.back(); }
auto anchor_stack::push(token_type token) 
    -> void {
    m_stack.push_back(token);
}
auto anchor_stack::push(std::span<token_type const> tokens)
    -> void {
    std::ranges::for_each(tokens, LIFT_MEMBER(push));
}
auto anchor_stack::pop(token_type token)
    -> void {
    if (m_stack.size() <= 1) {
        throw internal_error("Anchor stack reached bottom");
    } 
    if (m_stack.back() != token) {
        throw internal_error(
            fmt::format(
                "Anchor missmatch:\nexpected: {}\nstack: {}",
                to_string(token),
                m_stack|std::views::transform(LIFT(to_string))
            )
        );
    }
    m_stack.pop_back();
}
auto anchor_stack::pop(std::span<token_type const> tokens)
    -> void {
    std::ranges::for_each(tokens, LIFT_MEMBER(pop));
}



}
