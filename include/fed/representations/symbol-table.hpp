#ifndef FED_SYMBOL_TABLE_HPP_
#define FED_SYMBOL_TABLE_HPP_

#include <string_view>
#include <vector>
#include <unordered_map>
#include <utility>

#include "fed/diagnostics/internal-error.hpp"


namespace fed {

enum class symbol_type {
    function,
    variable,
    type,
    enum_,
    constant,
};
enum class naked_id : unsigned {};
struct symbol {
    naked_id id;
    symbol_type type;
};
class scope {
public:
    using map = std::unordered_map<std::string, symbol>;
    using iterator = map::iterator;
    auto add_symbol(std::string_view, symbol)
        -> iterator;
    auto lookup(std::string_view) const
        -> std::optional<symbol>;
    auto add_new_scope()
        -> scope&;
    auto get_parent()
        -> scope&;
    auto is_global_scope()
        -> bool;
private:
    std::unordered_map<std::string, naked_id> m_table;
    std::vector<scope> m_child_scopes;
    scope* m_parent;
};

template<typename AstT>
class symbol_mapback {
public:
    enum class id { poison = 0 };
    auto add_symbol(AstT* node)
        -> id { 
        m_map.push_back(node);
        return static_cast<id>(m_map.size() - 1);
    }
    auto lookup(id id)
        -> AstT* {
        if (id == id::poison) throw fed::internal_error("symtable");
        auto const index = std::to_underlying(id);
        if (index >= m_map.size()) throw fed::internal_error("symtable");
        return m_map[index];
    }
private:
    std::vector<AstT*> m_map;
};

} // namespace fed

#endif
