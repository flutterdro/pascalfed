#include "fed/scanner/token.hpp"
#include <vector>

namespace fed {

class token_queue {
public:
    using buffer_type = std::vector<token_view>;
    using iterator = buffer_type::iterator;
    using const_iterator = buffer_type::const_iterator;

    constexpr auto empty() const noexcept
        -> bool { return m_buffer.empty(); }
    constexpr auto push(token_view tok)
        -> void { m_buffer.push_back(tok); }
    constexpr auto pop()
        -> void { m_buffer.erase(m_buffer.begin()); }
    constexpr auto front() const 
        -> token_view { return m_buffer.front(); }
    constexpr auto back() const
        -> token_view { return m_buffer.back(); }
    constexpr auto begin() const 
        -> const_iterator { return m_buffer.begin(); }
    constexpr auto end() const
        -> const_iterator { return m_buffer.end(); }
    constexpr auto size() const
        -> std::size_t { return m_buffer.size(); }
    constexpr auto clear()
        -> void { m_buffer.clear(); }
    constexpr auto operator[](std::size_t index)
        -> token_view { return m_buffer[index]; }

private:
    std::vector<token_view> m_buffer;
};
}
