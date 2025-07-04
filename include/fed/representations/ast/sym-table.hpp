#ifndef FED_SYM_TABLE_HPP_SJDNCKNDC
#define FED_SYM_TABLE_HPP_SJDNCKNDC

#include "fed/representations/ast/forward.hpp"
#include "fed/representations/ast/handle.hpp"

namespace fed::ast {

class symbol_table {
public:
    constexpr symbol_table() noexcept = default;
    constexpr symbol_table(symbol_table&&) noexcept = default;
    constexpr symbol_table(symbol_table const&) = delete;

    constexpr auto operator=(symbol_table&&) noexcept
        -> symbol_table& = default;
    constexpr auto operator=(symbol_table const&)
        -> symbol_table& = delete;

    auto add(type_declaration) 
        -> type_id;
    auto add(variable_declaration) 
        -> variable_id;
    auto add(constant_declaration) 
        -> constant_id;
    auto add(function_declaration) 
        -> function_id;
    auto add(procedure_declaration) 
        -> procedure_id;

    auto get(type_id) const
        -> observer_handle<type_declaration>;
    auto get(variable_id) const
        -> observer_handle<variable_declaration>;
    auto get(constant_id) const
        -> observer_handle<constant_declaration>;
    auto get(function_id) const
        -> observer_handle<function_declaration>;
    auto get(procedure_id) const
        -> observer_handle<procedure_declaration>;
private:
    std::vector<type_declaration>      m_types;
    std::vector<variable_declaration>  m_variables;
    std::vector<constant_declaration>  m_constants;
    std::vector<function_declaration>  m_functions;
    std::vector<procedure_declaration> m_procedures;
};

}




#endif
