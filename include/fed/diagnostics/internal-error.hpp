#ifndef FED_INTERNAL_ERROR_HPP_
#define FED_INTERNAL_ERROR_HPP_

#include <chrono>
#include <cpptrace/basic.hpp>
#include <fmt/base.h>
#include <source_location>
#include <stdexcept>
#include <cpptrace/cpptrace.hpp>
#include <cpptrace/formatting.hpp>
#include <fmt/core.h>
#include <fmt/std.h>

namespace fed {

// this bad boy is bloated out of this world
class internal_error : public cpptrace::runtime_error {
public:
    using base_exception  = cpptrace::runtime_error;
    using source_location = std::source_location;
    using stacktrace      = cpptrace::stacktrace;
    using time            = std::chrono::time_point<std::chrono::system_clock>;
    internal_error(
        std::string&& message,
        source_location location = std::source_location::current(),
        time now = time::clock::now()
    ) : base_exception(std::move(message)), m_location(location), m_time_point(now) {}
    auto where() const noexcept
        -> source_location const& { return m_location; }
    auto when() const noexcept 
        -> time { return m_time_point; }
    auto how() const noexcept
        -> stacktrace const& { return base_exception::trace(); }
private:
    source_location m_location;
    time            m_time_point;
};

}

template<>
struct fmt::formatter<fed::internal_error> {
    constexpr auto parse(fmt::format_parse_context& ctx) {
        return ctx.begin();
    }
    auto format(fed::internal_error const& err, fmt::format_context& ctx) const {
        return fmt::format_to(ctx.out(), 
            "[{:%Y-%m-%d %H:%M:%S}] "
            "Encountered severe internal error with message:\n{}\n\n"
            "Thrown from here:\n{}\n\n"
            "Printing stacktrace:\n{}\n",
            std::chrono::floor<std::chrono::seconds>(err.when()), 
            err.message(), 
            err.where(), 
            cpptrace::formatter{}.format(err.how())
        );
    }
};

#endif // !FED_INTERNAL_ERROR_HPP_
