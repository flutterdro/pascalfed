#include <fmt/base.h>
#include <utility>
#include "fed/representations/ast.hpp"
#include "fed/representations/ast/nodes.hpp"
#include "fed/representations/ast/pretty-print.hpp"

using namespace fed;
auto fmt::formatter<ast::expression>::format(
    ast::expression const& val, 
    fmt::format_context& ctx
) const -> fmt::format_context::iterator {
    return std::visit([&]<typename T>(T const& v) {
        return fmt::formatter<T>{indentable::same_level()}
            .format(v, ctx);
    }, val);
}

auto fmt::formatter<ast::binary_expression>::format(
    ast::binary_expression const& val, 
    fmt::format_context& ctx
)  const -> fmt::format_context::iterator {
    auto binop_tostr = [](ast::binary_operation op) {
        switch (op) {
            case ast::binary_operation::add: return "(+)";
            case ast::binary_operation::and_: return "(and)";
            case ast::binary_operation::equal: return "(=)";
            case ast::binary_operation::substract: return "(-)";
            case ast::binary_operation::or_: return "(or)";
            case ast::binary_operation::multiply: return "(*)";
            case ast::binary_operation::integer_divide: return "(div)";
            case ast::binary_operation::real_divide: return "(/)";
            case ast::binary_operation::modulo: return "(mod)";
            case ast::binary_operation::not_equal: return "(<>)";
            case ast::binary_operation::greater: return "(>)";
            case ast::binary_operation::less: return "(<)";
            case ast::binary_operation::greater_or_equal: return "(>=)";
            case ast::binary_operation::less_or_equal: return "(<=)";
            case ast::binary_operation::in: return "(in)";
        }

    };
    ctx.out() = indentable::indent(ctx);
    ctx.out() = fmt::format_to(
        ctx.out(), 
        "binary expression: {}\n", 
        binop_tostr(val.operation)
    );
    ctx.out() = 
        fmt::formatter<ast::handle<ast::expression>>{indentable::new_level()}
            .format(val.lhs, ctx);
    ctx.out()++ = '\n';
    ctx.out() = 
        fmt::formatter<ast::handle<ast::expression>>{indentable::new_level()}
            .format(val.rhs, ctx);
    return ctx.out();
}

auto fmt::formatter<ast::unary_expression>::format(
    ast::unary_expression const& val, 
    fmt::format_context& ctx
) const -> fmt::format_context::iterator {
    ctx.out() = indentable::indent(ctx);
    return fmt::format_to(ctx.out(), "to be implemented");
}

auto fmt::formatter<ast::expression_leaf>::format(
    ast::expression_leaf const& val, 
    fmt::format_context& ctx
) const -> fmt::format_context::iterator {
    return std::visit([&]<typename T>(T const& v) {
        return fmt::formatter<T>{indentable::same_level()}
            .format(v, ctx);
    }, val);
}

auto fmt::formatter<ast::dereferenced_expression>::format(
    ast::dereferenced_expression const& val, 
    fmt::format_context& ctx
) const -> fmt::format_context::iterator {
    ctx.out() = indentable::indent(ctx);
    return fmt::format_to(ctx.out(), "to be implemented");
}
auto fmt::formatter<ast::called_expression>::format(
    ast::called_expression const& val, 
    fmt::format_context& ctx
) const -> fmt::format_context::iterator {
    ctx.out() = indentable::indent(ctx);
    return fmt::format_to(ctx.out(), "to be implemented");
}

auto fmt::formatter<ast::indexed_expression>::format(
    ast::indexed_expression const& val, 
    fmt::format_context& ctx
) const -> fmt::format_context::iterator {
    ctx.out() = indentable::indent(ctx);
    return fmt::format_to(ctx.out(), "to be implemented");
}

auto fmt::formatter<ast::membered_expression>::format(
    ast::membered_expression const& val, 
    fmt::format_context& ctx
) const -> fmt::format_context::iterator {
    ctx.out() = indentable::indent(ctx);
    return fmt::format_to(ctx.out(), "to be implemented");
}

auto fmt::formatter<ast::expression_atom>::format(
    ast::expression_atom const& val, 
    fmt::format_context& ctx
) const -> fmt::format_context::iterator {
    return std::visit([&]<typename T>(T const& v) {
        return fmt::formatter<T>{indentable::same_level()}
            .format(v, ctx);
    }, val);
}


auto fmt::formatter<ast::variable_name>::format(
    ast::variable_name const& val, 
    fmt::format_context& ctx
) const -> fmt::format_context::iterator {
    ctx.out() = indentable::indent(ctx);
    return fmt::format_to(ctx.out(), "variable id: {}", std::to_underlying(val.id));
}

auto fmt::formatter<ast::function_name>::format(
    ast::function_name const& val, 
    fmt::format_context& ctx
) const -> fmt::format_context::iterator {
    ctx.out() = indentable::indent(ctx);
    return fmt::format_to(ctx.out(), "to be implemented");
}

auto fmt::formatter<ast::constant>::format(
    ast::constant const& val, 
    fmt::format_context& ctx
) const -> fmt::format_context::iterator {
    ctx.out() = indentable::indent(ctx);
    return fmt::format_to(ctx.out(), "to be implemented");
}

auto fmt::formatter<ast::type>::format(
    ast::type const& val, 
    fmt::format_context& ctx
) const -> fmt::format_context::iterator {
    return std::visit([&]<typename T>(T const& v) {
        return fmt::formatter<T>{indentable::same_level()}
            .format(v, ctx);
    }, val);
}

auto fmt::formatter<ast::type_identifier>::format(
    ast::type_identifier const& val, 
    fmt::format_context& ctx
) const -> fmt::format_context::iterator {
    ctx.out() = indentable::indent(ctx);
    return fmt::format_to(ctx.out(), "type id: {}", std::to_underlying(val.id));
}
auto fmt::formatter<ast::pointer_type>::format(
    ast::pointer_type const& val,
    fmt::format_context& ctx
) const -> fmt::format_context::iterator {
    ctx.out() = indentable::indent(ctx);
    ctx.out() = fmt::format_to(ctx.out(), "pointer to:\n");
    return fmt::formatter<ast::handle<ast::type>>{indentable::new_level()}
            .format(val.base, ctx);
}

auto fmt::formatter<ast::array_type>::format(
    ast::array_type const& val, 
    fmt::format_context& ctx
) const -> fmt::format_context::iterator {
    ctx.out() = indentable::indent(ctx);
    ctx.out() = fmt::format_to(ctx.out(), "array of:\n");
    ctx.out() = fmt::formatter<ast::handle<ast::type>>{indentable::new_level()}
            .format(val.component_type, ctx);
    ctx.out()++ = '\n';

    ctx.out() = indentable::new_level().indent(ctx);
    ctx.out() = fmt::format_to(ctx.out(), "indexed with:");
    for (auto const& index_type : val.index_types) {
        ctx.out()++ = '\n';
        ctx.out() = fmt::formatter<ast::handle<ast::type>>{
            indentable::new_level().new_level()
        }.format(index_type, ctx);
    }
    return ctx.out();
}
auto fmt::formatter<ast::function_type>::format(
    ast::function_type const& val, 
    fmt::format_context& ctx
) const -> fmt::format_context::iterator {
    ctx.out() = indentable::indent(ctx);
    return fmt::format_to(ctx.out(), "to be implemented");
}
auto fmt::formatter<ast::file_type>::format(
    ast::file_type const& val, 
    fmt::format_context& ctx
) const -> fmt::format_context::iterator {
    ctx.out() = indentable::indent(ctx);
    return fmt::format_to(ctx.out(), "to be implemented");
}
auto fmt::formatter<ast::set_type>::format(
    ast::set_type const& val, 
    fmt::format_context& ctx
) const -> fmt::format_context::iterator {
    ctx.out() = indentable::indent(ctx);
    return fmt::format_to(ctx.out(), "to be implemented");
}
auto fmt::formatter<ast::record_type>::format(
    ast::record_type const& val, 
    fmt::format_context& ctx
) const -> fmt::format_context::iterator {
    ctx.out() = indentable::indent(ctx);
    return fmt::format_to(ctx.out(), "to be implemented");
}
auto fmt::formatter<ast::enumerated_type>::format(
    ast::enumerated_type const& val, 
    fmt::format_context& ctx
) const -> fmt::format_context::iterator {
    ctx.out() = indentable::indent(ctx);
    return fmt::format_to(ctx.out(), "to be implemented");
}
auto fmt::formatter<ast::subrange_type>::format(
    ast::subrange_type const& val, 
    fmt::format_context& ctx
) const -> fmt::format_context::iterator {
    ctx.out() = indentable::indent(ctx);
    return fmt::format_to(ctx.out(), "to be implemented");
}
auto fmt::formatter<ast::type_builtin>::format(
    ast::type_builtin const& val, 
    fmt::format_context& ctx
) const -> fmt::format_context::iterator {
    ctx.out() = indentable::indent(ctx);
    return fmt::format_to(ctx.out(), "to be implemented");
}
auto fmt::formatter<ast::procedure_type>::format(
    ast::procedure_type const& val, 
    fmt::format_context& ctx
) const -> fmt::format_context::iterator {
    ctx.out() = indentable::indent(ctx);
    return fmt::format_to(ctx.out(), "to be implemented");
}
