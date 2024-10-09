#ifndef FED_DIAGNOSTICS_BUFFER_HPP_
#define FED_DIAGNOSTICS_BUFFER_HPP_

#include "fed/diagnostics/compile-error.hpp"
#include <memory>
#include <string_view>
#include <vector>

namespace fed {

class diagnostics_buffer {
public:
    auto output_message(std::string_view message)
        -> void;
    auto push_back(compilation_error) -> void;
    auto flush() -> void;

    [[noreturn]] auto too_many_errors_handler() noexcept
        -> void;

    unsigned m_error_count;
    unsigned m_max_error_count;
    std::vector<compilation_error> m_buffered_errors;
};


} // namespace fed 



#endif
