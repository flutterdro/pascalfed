#include <fmt/base.h>
#include <fmt/core.h>

#include "fed/representations/parse-tree.hpp"
#include "fed/utils/superutil.hpp"

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

auto fmt::formatter<ast::dereferenced_variable>::format(
    ast::dereferenced_variable const& val, 
    fmt::format_context& ctx
) const -> fmt::format_context::iterator {
    ctx.out() = indentable::indent(ctx);
    return fmt::format_to(ctx.out(), "to be implemented");
}
auto fmt::formatter<ast::called_variable>::format(
    ast::called_variable const& val, 
    fmt::format_context& ctx
) const -> fmt::format_context::iterator {
    ctx.out() = indentable::indent(ctx);
    return fmt::format_to(ctx.out(), "to be implemented");
}

auto fmt::formatter<ast::indexed_variable>::format(
    ast::indexed_variable const& val, 
    fmt::format_context& ctx
) const -> fmt::format_context::iterator {
    ctx.out() = indentable::indent(ctx);
    return fmt::format_to(ctx.out(), "to be implemented");
}

auto fmt::formatter<ast::membered_variable>::format(
    ast::membered_variable const& val, 
    fmt::format_context& ctx
) const -> fmt::format_context::iterator {
    ctx.out() = indentable::indent(ctx);
    return fmt::format_to(ctx.out(), "to be implemented");
}

auto fmt::formatter<ast::bare_name>::format(
    ast::bare_name const& val, 
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



