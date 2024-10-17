#include "fed/parser/context.hpp"
#include "fed/representations/parse-tree.hpp"
#include "fed/representations/symbol-table.hpp"
#include "fed/utils/superutil.hpp"
#include <algorithm>
#include <array>
#include <optional>
#include <string_view>


namespace fed {

struct type_checker {
    auto operator()(ast::type const& lhs, ast::type const& rhs)
        -> check_result {
        return std::visit(*this, lhs, rhs);
    }
    // while type definition itself was/(should be) previously checked for poisoning
    // it still could be an alias to the poisoned type 
    // if it is the case act as if the check was success 
    // the same rational as usual
    auto operator()(ast::type_identifier const& lhs, ast::type_identifier const& rhs)
        -> check_result {
        if (std::ranges::any_of(std::array{lhs.id, rhs.id}, func::equal_to(sym::type::id::poison))) {
            return check_result::success;
        }
        if (lhs.id == rhs.id) return check_result::success;
        else return check_result::failure;
    }
    auto operator()(ast::type_identifier const& lhs, auto const& rhs)
        -> check_result {
        if (lhs.id == sym::type::id::poison) return check_result::success;
        return (*this)(ctx.lookup(lhs.id)->types, rhs);
    }
    auto operator()(auto const& lhs, ast::type_identifier const& rhs)
        -> check_result {
        return (*this)(rhs, lhs);
    }
    auto operator()(ast::subrange_type const& lhs, ast::subrange_type const& rhs)
        -> check_result {
        if (*lhs.begin == *rhs.begin and *lhs.end == *rhs.end) {
            return check_result::success;
        } else {
            return check_result::failure;
        }
    }
    auto operator()(ast::array_type const& lhs, ast::array_type const& rhs)
        -> check_result {
        if (lhs.index_types.size() != rhs.index_types.size()) return check_result::failure;
        for (auto i = 0uz; i < lhs.index_types.size(); ++i) {
            if ((*this)(lhs.index_types[i], rhs.index_types[i]) == check_result::failure) {
                return check_result::failure;
            }
        }
        return (*this)(lhs.component_type, rhs.component_type);
    }
    template<typename T, typename F>
    auto operator()(T const&, F const&) 
        -> check_result { return check_result::failure; }

    semantic_context& ctx;
};

auto semantic_context::type_check(sym::type::info lhs, sym::type::info rhs)
    -> check_result {
    auto const& type_lhs = lhs->types;
    auto const& type_rhs = rhs->types;

    // if any of the type handles are poisoned
    // always act as if type check was successful and move compiler forward
    // otherwise there would be too many trivial and noisy error messages which
    // would hide a true error
    if (type_rhs.is_poisoned() or type_lhs.is_poisoned()) {
        return check_result::success;
    }

    return type_checker{*this}(type_lhs, type_rhs);
}

auto semantic_context::insert(std::string_view name, sym::type::info info)
    -> std::optional<contextual_error> {
    auto& current_scope = m_current_scope.scope();
    // check if type declaration is is poisoned 
    // in case it is asign this type a poison id 
    // in case it isn't go through the usual motions
    // if provided type is an alias to existing type, just reuse the id 
    // otherwise create a new id
    auto type_id = info->types.is_poisoned() ? sym::type::id::poison :
    std::visit(overloaded{
        [](ast::type_identifier const& type) { return type.id; },
        [&](auto const& type) { return m_symbol_table.types().insert(info); }
    }, *info->types);
    auto [iterator, success] = current_scope.insert(
        name, 
        {
            .type = sym::name_type::type, 
            .id = static_cast<unsigned>(type_id)
        }
    );

    if (success) { 
        return std::nullopt; 
    } else {
        return contextual_error();
    }
}

auto semantic_context::insert(std::string_view name, sym::variable::info info)
    -> std::optional<contextual_error> {
    auto& current_scope = m_current_scope.scope();
    auto variable_id = m_symbol_table.variables().insert(info);
    auto [iterator, success] = current_scope.insert(
        name, 
        {
            .type = sym::name_type::variable, 
            .id = static_cast<unsigned>(variable_id)
        }
    );

    if (success) { 
        return std::nullopt; 
    } else {
        return contextual_error();
    }
}

auto semantic_context::insert(std::string_view name, sym::function::info info)
    -> std::optional<contextual_error> {
    auto& current_scope = m_current_scope.scope();
    auto function_id = m_symbol_table.functions().insert(info);
    auto [iterator, success] = current_scope.insert(
        name, 
        {
            .type = sym::name_type::function, 
            .id = static_cast<unsigned>(function_id)
        }
    );

    if (success) { 
        return std::nullopt; 
    } else {
        return contextual_error();
    }
}

auto semantic_context::insert(std::string_view name, sym::constant::info info)
    -> std::optional<contextual_error> {
    auto& current_scope = m_current_scope.scope();
    auto constant_id = m_symbol_table.constants().insert(info);
    auto [iterator, success] = current_scope.insert(
        name, 
        {
            .type = sym::name_type::constant, 
            .id = static_cast<unsigned>(constant_id)
        }
    );

    if (success) { 
        return std::nullopt; 
    } else {
        return contextual_error();
    }
}

auto semantic_context::lookup_type(std::string_view name)
    -> lookup_result<sym::type::id> {

    for (auto walker = m_current_scope; 
        walker != m_scoped_names.get_walker();
        walker.backtrack_to_parent()) {
        auto& scope = walker.scope();
        if (auto it = scope.lookup(name);
            it != scope.end()) {
            auto [type, id] = it->second;
            if (type != sym::name_type::type) {
                return std::unexpected(contextual_error());
            } else {
                return static_cast<sym::type::id>(id);
            }
        }
    }

    return std::unexpected(contextual_error());
}

auto semantic_context::lookup_constant(std::string_view name) 
    -> lookup_result<sym::constant::id> {
    
    for (auto walker = m_current_scope; 
        walker != m_scoped_names.get_walker();
        walker.backtrack_to_parent()) {
        auto& scope = walker.scope();
        if (auto it = scope.lookup(name);
            it != scope.end()) {
            auto [type, id] = it->second;
            if (type != sym::name_type::constant) {
                return std::unexpected(contextual_error());
            } else {
                return static_cast<sym::constant::id>(id);
            }
        }
    }

    return std::unexpected(contextual_error());
}

auto semantic_context::lookup(sym::type::id id) const
    -> sym::type::info { return m_symbol_table.types().lookup(id); }
auto semantic_context::lookup(sym::constant::id id) const
    -> sym::constant::info { return m_symbol_table.constants().lookup(id); }
auto semantic_context::initialize_scope() 
    -> void {}
auto semantic_context::finalize_scope()
    -> void {}

} // namespace fed
