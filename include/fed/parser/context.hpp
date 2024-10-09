#ifndef FED_SEMANTIC_CONTEXT_HPP_
#define FED_SEMANTIC_CONTEXT_HPP_

#include "fed/representations/parse-tree.hpp"
#include "fed/representations/symbol-table.hpp"
#include "fed/parser/semantic-error.hpp"
#include <expected>

namespace fed {

enum class check_result {
    success = 0,
    failure = 1,
};
template<typename T>
using lookup_result = std::expected<T, contextual_error>;
class semantic_context {
public:
    semantic_context();
    auto initialize_scope()
        -> void;
    auto lookup_type(std::string_view name) 
        -> lookup_result<sym::type::id>;
    auto lookup_variable(std::string_view name) const
        -> void;
    auto lookup_function(std::string_view name) const
        -> void;
    auto lookup_constant(std::string_view name) 
        -> lookup_result<sym::constant::id>; 
    auto lookup(sym::type::id) const
        -> sym::type::info;
    auto lookup(sym::variable::id) const;
    auto lookup(sym::function::id) const;
    auto lookup(sym::constant::id) const
        -> sym::constant::info;
    auto insert(std::string_view name, sym::type::info) 
        -> std::optional<contextual_error>;
    auto insert(std::string_view name, sym::variable::info) 
        -> std::optional<contextual_error>;
    auto insert(std::string_view name, sym::function::info) 
        -> std::optional<contextual_error>;
    auto insert(std::string_view name, sym::constant::info) 
        -> std::optional<contextual_error>;
    auto finalize_scope()
        -> void;
    auto plunder_symbol_table()
        -> sym::table;

    auto type_check(sym::type::info lhs, sym::type::info rhs)
        -> check_result; 
private:
    sym::table              m_symbol_table;
    sym::scope_tree         m_scoped_names;
    sym::scope_tree::walker m_current_scope;
};


} // namespace fed
#endif
