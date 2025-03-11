#include "fed/representations/symbol-table.hpp"
#include <optional>
#include <variant>


namespace fed {

auto scope::lookup(std::string_view name) const
    -> std::optional<symbol> {
    auto it = m_table.find(name);
    if (it != m_table.end()) return it->second;
    return std::nullopt;

}


};
