#ifndef FED_SYMBOL_TABLE_HPP_
#define FED_SYMBOL_TABLE_HPP_

#include <fmt/base.h>
#include <source_location>
#include <string_view>
#include <vector>
#include <unordered_map>
#include <utility>
#include <fmt/core.h>
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
    scope() = default;
    scope(scope const&) = delete;
    scope(scope&&) = default;
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
    std::vector<std::unique_ptr<scope>> m_child_scopes;
    scope* m_parent;
};

template<typename AstT>
class symbol_mapback {
public:
    enum class id { poison = 0 };
    auto add_symbol(AstT node)
        -> id { 
        m_map.push_back(std::move(node));
        return static_cast<id>(m_map.size() - 1);
    }
    auto lookup(id id, std::source_location loc = std::source_location::current()) const
        -> AstT const& {
        auto const index = std::to_underlying(id);
        if (index >= m_map.size()) throw fed::internal_error("symtable", loc);
        return m_map[index];
    }
    auto id_to_be() const noexcept
        -> id { return static_cast<id>(m_map.size()); }
private:
    std::vector<AstT> m_map;
};
inline auto format_as(fed::naked_id id) {
    return fmt::underlying(id);
}

} // namespace fed

template<>
struct fmt::formatter<fed::symbol_type>  {
    constexpr auto parse(fmt::format_parse_context& ctx) {
        return ctx.begin();
    }
    auto format(fed::symbol_type const& type, fmt::format_context& ctx) const {
        switch (type) {
            case fed::symbol_type::function:
                return fmt::format_to(ctx.out(), "function");
            case fed::symbol_type::variable:
                return fmt::format_to(ctx.out(), "variable");
            case fed::symbol_type::type:    
                return fmt::format_to(ctx.out(), "type");
            case fed::symbol_type::enum_:   
                return fmt::format_to(ctx.out(), "enum_");
            case fed::symbol_type::constant:
                return fmt::format_to(ctx.out(), "constant");
        }
    }
};
template<>
struct fmt::formatter<fed::symbol>  {
    constexpr auto parse(fmt::format_parse_context& ctx) {
        return ctx.begin();
    }
    auto format(fed::symbol const& type, fmt::format_context& ctx) const {
        return fmt::format_to(ctx.out(), "{} with id: {}", type.type, std::to_underlying(type.id));
    }
};


#endif
