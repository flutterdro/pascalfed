#include "fed/representations/raw-source.hpp"
#include <iterator>

namespace fed::source {

view::view(iterator begin, iterator end) noexcept
    : m_view(begin.base(), end.base()), m_marker(begin.where()) {}

full_view::full_view(underlying_view_t view)
    : m_view{view} {}

auto iterator::operator++() noexcept
    -> iterator& {
    if (*m_iterator == '\n') {
        m_location.column = 0;
        ++m_location.line;
    } else {
        ++m_location.column;
    }
    ++m_iterator;

    return *this;
}

auto iterator::operator++(int) noexcept
    -> iterator {
    auto tmp = *this;
    ++*this;

    return tmp;
}

auto iterator::operator*() const noexcept
    -> value_type {
    return *m_iterator;
}

auto full_view::begin() const noexcept
    -> iterator {
    return {
        m_view.begin(),
        location{
            .line   = 0,
            .column = 0,
        }
    };
}

auto full_view::end() const noexcept
    -> sentinel { return m_view.end(); }

auto full_view::subview(iterator start, iterator end) const noexcept
    -> view { 
    return  view{
        start,
        end
    }; 
}

static_assert(std::forward_iterator<full_view::iterator>);

} // namespace fed::source
