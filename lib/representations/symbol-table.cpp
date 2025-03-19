#include "fed/representations/symbol-table.hpp"
#include <optional>
#include <string_view>
#include <variant>
#include <fmt/std.h>
#include <fmt/ranges.h>


namespace fed {

auto scope::lookup(std::string_view name) const
    -> std::optional<symbol> {
    auto it = m_table.find(name);
    if (it != m_table.end()) return it->second;
    return std::nullopt;

}
auto scope::add_symbol(std::string_view name, symbol sym)
    -> iterator {
    auto res = m_table.insert({std::string(name), sym});
    
    return res.first;
}
auto scope::is_free_real_estate(std::string_view name) const
    -> bool {
    return not m_table.contains(name);
}


};
