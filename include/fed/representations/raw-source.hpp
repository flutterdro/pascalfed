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

struct storage {
    
};

struct view {
    constexpr view() noexcept = default;
    view(location marker, std::string_view view) noexcept;
    auto subview(std::size_t start, std::size_t count) const noexcept
        -> view;
    location m_marker;
    std::string_view m_view;
};

class full_view {
public:
    class iterator;
    using sentinel = std::string_view::iterator;

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
    std::string_view m_view; 
};

class full_view::iterator {
public:
    using difference_type = std::ptrdiff_t;
    using value_type      = char;
    using base_iterator   = std::string_view::iterator;
    
    iterator() noexcept = default;
    iterator(base_iterator base, location loc) noexcept;

    auto operator==(std::string_view::iterator sentinel) const noexcept
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
        -> location;
    auto base() const noexcept
        -> base_iterator;
private:
    base_iterator m_iterator{};
    location m_location{};
};


} // namespace source 


} // namespace fed

#endif
