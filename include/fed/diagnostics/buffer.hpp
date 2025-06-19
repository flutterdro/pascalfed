#ifndef FED_DIAGNOSTICS_BUFFER_HPP_
#define FED_DIAGNOSTICS_BUFFER_HPP_

#include "fed/diagnostics/compile-error.hpp"
#include <memory>
#include <string_view>
#include <vector>

namespace fed {

class diagnostics_buffer {
public:
    diagnostics_buffer() = default;
    auto output_message(std::string_view message)
        -> void;
    auto push_back(compilation_error) -> void;
    auto flush() -> void;
    auto current_error_count()
        -> std::size_t;

    [[noreturn]] auto too_many_errors_handler() noexcept
        -> void;

    unsigned m_error_count{};
    unsigned m_max_error_count{5};
    std::vector<compilation_error> m_buffered_errors{};
};


} // namespace fed 


#endif
