#include "fed/diagnostics/buffer.hpp"
#include "fed/diagnostics/compile-error.hpp"

#include <cstdlib>
#include <print>


namespace fed {

auto diagnostics_buffer::push_back(compilation_error error) 
    -> void {
    if (++m_error_count >= m_max_error_count) too_many_errors_handler();
    m_buffered_errors.push_back(std::move(error));
}

auto diagnostics_buffer::flush()
    -> void {
    for(auto const& error : m_buffered_errors) {
        std::println("{}", error.message());
    }
    m_buffered_errors.clear();
}

auto diagnostics_buffer::too_many_errors_handler() noexcept
    -> void {
    std::println("fatal error: too many errors");
    flush();
    std::exit(EXIT_FAILURE);
}


} //namespace fed
