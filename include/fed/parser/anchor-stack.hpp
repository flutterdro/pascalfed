#ifndef FED_ANCHOR_STACK_HPP_ASNZJPOVIDJ
#define FED_ANCHOR_STACK_HPP_ASNZJPOVIDJ

#include "fed/scanner/token.hpp"
#include <span>
#include <vector>

namespace fed {

class anchor_stack {
public:
    anchor_stack();
    anchor_stack(anchor_stack const&) = default;
    anchor_stack(anchor_stack&&) noexcept = default;

    auto operator=(anchor_stack const&) 
        -> anchor_stack& = default;
    auto operator=(anchor_stack&&) noexcept
        -> anchor_stack& = default;

    auto push(token_type)
        -> void;
    auto push(std::span<token_type const>)
        -> void;
    // pop accepts a token to match with the popped
    // value. mismatch must not and mostly will never happen 
    // but if it does it will be detected
    auto pop(token_type)
        -> void;
    auto pop(std::span<token_type const>)
        -> void;
    auto top() const noexcept
        -> token_type;

    auto begin() const {
        return m_stack.crbegin();
    }
    auto end() const {
        return m_stack.crend();
    }
private:
    std::vector<fed::token_type> m_stack;
};

}

#endif // !FED_ANCHOR_STACK_HPP_ASNZJPOVIDJ
