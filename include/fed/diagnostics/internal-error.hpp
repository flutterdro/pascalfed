#ifndef FED_INTERNAL_ERROR_HPP_
#define FED_INTERNAL_ERROR_HPP_

#include <source_location>
#include <stdexcept>

namespace fed {

class internal_error : std::runtime_error {
public:
    internal_error(
        std::string const& message,
        std::source_location location = std::source_location::current()
    );
    auto where() const noexcept
        -> std::source_location;
private:
    std::source_location m_location;
};

}

#endif // !FED_INTERNAL_ERROR_HPP_
