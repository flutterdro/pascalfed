#ifndef FED_RAW_SOURCE_HPP_
#define FED_RAW_SOURCE_HPP_

#include <cstddef>
#include <string_view>
namespace fed {

namespace source {

struct location {
    unsigned line;
    unsigned column;
};

using underlying_view_t = std::string_view;
 
class iterator {
public:
    using difference_type = std::ptrdiff_t;
    using value_type      = char;
    using base_iterator   = underlying_view_t::iterator;
    
    iterator() noexcept = default;
    iterator(base_iterator base, location loc) noexcept
        : m_iterator(base), m_location(loc) {}

    auto operator==(base_iterator sentinel) const noexcept
        -> bool { return sentinel == m_iterator; }
    auto operator==(iterator other) const noexcept
        -> bool { return other.m_iterator == m_iterator; }
    auto operator++() noexcept
        -> iterator&;
    auto operator++(int) noexcept
        -> iterator;
    auto operator*() const noexcept
        -> value_type;

    auto where() const noexcept
        -> location { return m_location; }
    auto base() const noexcept
        -> base_iterator { return m_iterator; }
private:
    base_iterator m_iterator{};
    location m_location{};
};

struct storage {
    
};

struct view {

    using iterator = source::iterator;
    using sentinel = underlying_view_t::iterator;

    constexpr view() noexcept = default;
    view(iterator start, iterator end) noexcept;
    view(location marker, underlying_view_t view) noexcept;
    auto subview(std::size_t start, std::size_t count) const noexcept
        -> view;
    
    auto begin() const noexcept
        -> iterator;
    auto end() const noexcept
        -> sentinel;
    auto data() const noexcept
        -> char const*;
    auto size() const noexcept
        -> std::size_t { return m_view.size(); }
    auto base() const noexcept
        -> underlying_view_t { return m_view; }

    location m_marker;
    underlying_view_t m_view;
};

class full_view {
public:
    using iterator = source::iterator;
    using sentinel = std::string_view::iterator;

    full_view(underlying_view_t view);

    auto begin() const noexcept
        -> iterator;
    auto end() const noexcept
        -> sentinel;
    auto data() const noexcept
        -> char const*;
    auto size() const noexcept
        -> std::size_t;
    auto count_lines() const noexcept
        -> unsigned;
    auto subview(iterator start, iterator end) const noexcept
        -> view;
private:
    underlying_view_t m_view; 
};




} // namespace source 


} // namespace fed

#endif
