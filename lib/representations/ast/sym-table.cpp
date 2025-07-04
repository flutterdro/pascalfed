#include "fed/representations/ast/sym-table.hpp"
#include "fed/representations/ast/nodes.hpp"
#include <utility>


namespace fed::ast {

auto symbol_table::add(type_declaration decl) 
    -> type_id {
    m_types.push_back(std::move(decl));
    return static_cast<type_id>(m_types.size() - 1);
}
auto symbol_table::add(variable_declaration decl) 
    -> variable_id {
    m_variables.push_back(std::move(decl));
    return static_cast<variable_id>(m_variables.size() - 1);
}
auto symbol_table::add(constant_declaration decl) 
    -> constant_id {
    m_constants.push_back(std::move(decl));
    return static_cast<constant_id>(m_constants.size() - 1);
}
auto symbol_table::add(function_declaration decl) 
    -> function_id {
    m_functions.push_back(std::move(decl));
    return static_cast<function_id>(m_functions.size() - 1);
}
auto symbol_table::add(procedure_declaration decl) 
    -> procedure_id {
    m_procedures.push_back(std::move(decl));
    return static_cast<procedure_id>(m_procedures.size() - 1);
}

auto symbol_table::get(type_id id) const
    -> observer_handle<type_declaration> {
    auto const raw_id = std::to_underlying(id);
    if (m_types.size() <= raw_id) {
        throw internal_error("peepeepoopoo");
    }
    return m_types[raw_id];
}
auto symbol_table::get(variable_id id) const
    -> observer_handle<variable_declaration> {
    auto const raw_id = std::to_underlying(id);
    if (m_variables.size() <= raw_id) {
        throw internal_error("peepeepoopoo");
    }
    return m_variables[raw_id];
}
auto symbol_table::get(constant_id id) const
    -> observer_handle<constant_declaration> {
    auto const raw_id = std::to_underlying(id);
    if (m_constants.size() <= raw_id) {
        throw internal_error("peepeepoopoo");
    }
    return m_constants[raw_id];
}
auto symbol_table::get(function_id id) const
    -> observer_handle<function_declaration> {
    auto const raw_id = std::to_underlying(id);
    if (m_functions.size() <= raw_id) {
        throw internal_error("peepeepoopoo");
    }
    return m_functions[raw_id];
}
auto symbol_table::get(procedure_id id) const
    -> observer_handle<procedure_declaration> {
    auto const raw_id = std::to_underlying(id);
    if (m_procedures.size() <= raw_id) {
        throw internal_error("peepeepoopoo");
    }
    return m_procedures[raw_id];
}




}
