#include "fed/representations/symbol-table.hpp"
#include <optional>
#include <string_view>
#include <variant>


namespace fed {

auto scope::lookup(std::string_view name) const
    -> std::optional<symbol> {
    auto it = m_table.find(name);
    if (it != m_table.end()) return it->second;
    return std::nullopt;

}
auto scope::add_symbol(std::string_view name, symbol sym)
    -> iterator {
    return m_table.insert({std::string(name), sym}).first;
}
auto scope::is_free_real_estate(std::string_view name) const
    -> bool {
    return m_table.contains(name);
}


};
