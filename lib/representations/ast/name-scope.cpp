#include "fed/representations/ast/name-scope.hpp"
#include <fmt/ostream.h>
#include <optional>
#include "fed/diagnostics/internal-error.hpp"


namespace fed::ast {

name_scope::name_scope(name_scope const& parent, map_t map) noexcept
    : m_parent(&parent)
    , m_scope(std::move(map)) {}

auto name_scope::spawn() const
    -> std::unique_ptr<name_scope> { 
    return std::make_unique<name_scope>(*this, map_t()); 
}
auto name_scope::lookup(std::string_view name) const noexcept
    -> std::optional<tagged_id> {
    if (auto it = m_scope.find(name);
        it != m_scope.end()) {
        return it->second;
    }
    if (m_parent == nullptr) return std::nullopt;
    return m_parent->lookup(name);
}
auto name_scope::insert(std::string name, tagged_id id)
    -> insert_result {
    auto const [it, is_success] = m_scope.insert({
        std::move(name), id
    });
    return {
        .is_success = is_success,
        .insertee   = it,
    };
}

}
