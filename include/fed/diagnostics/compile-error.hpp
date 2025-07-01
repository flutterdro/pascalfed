#ifndef FED_COMILATION_ERROR_HPP_
#define FED_COMILATION_ERROR_HPP_

#include "fed/representations/raw-source.hpp"
#include "fed/utils/macros.hpp"

#include <fmt/format.h>
#include <string>
#include <memory>
#include <type_traits>

namespace fed {

template<typename T1, typename T2>
concept forward = std::same_as<std::remove_cvref_t<T1>, T2>;

class compilation_error {
    struct error_base {
        constexpr virtual auto tp_message() const 
            -> std::string = 0;
        constexpr virtual auto tp_location() const 
            -> source::location = 0;
        constexpr virtual ~error_base() = default;
    };
    
    template<typename T>
    struct error_holder : error_base {
        
        constexpr error_holder(forward<T> auto&& holder)
            : m_holder(FWD(holder)) {}
        constexpr auto tp_message() const 
            -> std::string override { return m_holder.message(); }
        constexpr auto tp_location() const 
            -> source::location override { return m_holder.location(); }
        T m_holder;
    };

    std::unique_ptr<error_base> m_error;
public:
    compilation_error(compilation_error const&) = delete;
    compilation_error(compilation_error&) = delete;
    constexpr compilation_error(compilation_error&&) noexcept = default;
    template<typename T>
    constexpr compilation_error(T&& error) 
        : m_error(std::make_unique<error_holder<std::decay_t<T>>>(FWD(error))) {}

    constexpr auto message() const
        -> std::string { 
        return fmt::format(
            "error at line {}:{}: {}", 
            m_error->tp_location().line,
            m_error->tp_location().column,
            m_error->tp_message()
        );
    }
    constexpr auto location() const
        -> source::location { return m_error->tp_location(); }
};


struct dummy_error {
    constexpr dummy_error(
        source::location loc,
        std::string message = "Feelin cute rn, might implement later")
        : m_location(loc), m_message(std::move(message)) {}
    constexpr auto message() const
        -> std::string { return m_message; }
    constexpr auto location() const
        -> source::location { return m_location; }
    std::string m_message;
    source::location m_location;
};


// class compilation_error {
// public:
//     virtual auto message() const 
//         -> std::string = 0;
//     virtual auto location() const
//         -> source::location;
//     virtual ~compilation_error() = default;
//
// };


}


#endif
