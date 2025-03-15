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
enum class naked_id : unsigned { poison = 0 };
constexpr auto strip(auto other_id) noexcept
    -> naked_id { return static_cast<naked_id>(std::to_underlying(other_id)); }
struct symbol {
    naked_id id;
    symbol_type type;
};
namespace detail {
struct string_hash
{
    using hash_type = std::hash<std::string_view>;
    using is_transparent = void;
 
    std::size_t operator()(const char* str) const        { return hash_type{}(str); }
    std::size_t operator()(std::string_view str) const   { return hash_type{}(str); }
    std::size_t operator()(std::string const& str) const { return hash_type{}(str); }
};
} // namespace detail 
template<typename T>
using name_map = std::unordered_map<std::string, T, detail::string_hash, std::equal_to<>>;
class scope {
public:
    using iterator = name_map<symbol>::iterator;
    auto add_symbol(std::string_view, symbol)
        -> iterator;
    auto is_free_real_estate(std::string_view) const
        -> bool;
    auto lookup(std::string_view) const
        -> std::optional<symbol>;
    auto add_new_scope()
        -> scope&;
    auto get_parent()
        -> scope&;
    auto is_global_scope()
        -> bool;
private:
    name_map<symbol> m_table;
    std::vector<scope> m_child_scopes;
    scope* m_parent;
};

template<typename AstT>
class symbol_mapback {
public:
    enum class id { poison = 0 };
    auto add_symbol(AstT node)
        -> id { 
        m_map.push_back(node);
        return static_cast<id>(m_map.size() - 1);
    }
    auto lookup(id id) const
        -> AstT {
        if (id == id::poison) throw fed::internal_error("symtable");
        auto const index = std::to_underlying(id);
        if (index >= m_map.size()) throw fed::internal_error("symtable");
        return m_map[index];
    }
private:
    std::vector<AstT> m_map;
};

} // namespace fed

#endif
