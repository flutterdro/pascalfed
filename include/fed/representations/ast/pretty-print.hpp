#ifndef FED_AST_PRETTY_PRINT_HPP_QIJSNIJN
#define FED_AST_PRETTY_PRINT_HPP_QIJSNIJN

#include <fmt/core.h>

#include "fed/representations/ast/nodes.hpp"
#include "fed/utils/superutil.hpp"
#include "fed/representations/ast/forward.hpp"


//
/// EXPRESSION PRETTY PRINT 
//
template<>
struct fmt::formatter<fed::ast::expression> : indentable {
    constexpr auto parse(fmt::format_parse_context& ctx) {
        return ctx.begin();
    }
    auto format(
        fed::ast::expression const& val, 
        fmt::format_context& ctx
    ) const -> fmt::format_context::iterator;
};
template<>
struct fmt::formatter<fed::ast::binary_expression> : indentable {
    constexpr auto parse(fmt::format_parse_context& ctx) {
        return ctx.begin();
    }
    auto format(fed::ast::binary_expression const&, fmt::format_context&) const
        -> fmt::format_context::iterator;
};
template<>
struct fmt::formatter<fed::ast::unary_expression> : indentable {
    unsigned level;
    constexpr auto parse(fmt::format_parse_context& ctx) {
        return ctx.begin();
    }
    auto format(fed::ast::unary_expression const&, fmt::format_context&) const
        -> fmt::format_context::iterator;
};
template<>
struct fmt::formatter<fed::ast::expression_leaf> : indentable {
    constexpr auto parse(fmt::format_parse_context& ctx) {
        return ctx.begin();
    }
    auto format(
        fed::ast::expression_leaf const& exp, 
        fmt::format_context& ctx
    ) const -> fmt::format_context::iterator;
};
template<>
struct fmt::formatter<fed::ast::dereferenced_expression> : indentable {
    constexpr auto parse(fmt::format_parse_context& ctx) {
        return ctx.begin();
    }
    auto format(
        fed::ast::dereferenced_expression const& exp, 
        fmt::format_context& ctx
    ) const -> fmt::format_context::iterator;
};
template<>
struct fmt::formatter<fed::ast::called_expression> : indentable {
    constexpr auto parse(fmt::format_parse_context& ctx) {
        return ctx.begin();
    }
    auto format(
        fed::ast::called_expression const& exp, 
        fmt::format_context& ctx
    ) const -> fmt::format_context::iterator;
};
template<>
struct fmt::formatter<fed::ast::indexed_expression> : indentable {
    constexpr auto parse(fmt::format_parse_context& ctx) {
        return ctx.begin();
    }
    auto format(
        fed::ast::indexed_expression const& exp, 
        fmt::format_context& ctx
    ) const -> fmt::format_context::iterator;
};
template<>
struct fmt::formatter<fed::ast::membered_expression> : indentable {
    constexpr auto parse(fmt::format_parse_context& ctx) {
        return ctx.begin();
    }
    auto format(
        fed::ast::membered_expression const& exp, 
        fmt::format_context& ctx
    ) const -> fmt::format_context::iterator;
};
template<>
struct fmt::formatter<fed::ast::expression_atom> : indentable {
    constexpr auto parse(fmt::format_parse_context& ctx) {
        return ctx.begin();
    }
    auto format(
        fed::ast::expression_atom const& exp, 
        fmt::format_context& ctx
    ) const -> fmt::format_context::iterator;
};
template<>
struct fmt::formatter<fed::ast::variable_name> : indentable {
    constexpr auto parse(fmt::format_parse_context& ctx) {
        return ctx.begin();
    }
    auto format(
        fed::ast::variable_name const& exp, 
        fmt::format_context& ctx
    ) const -> fmt::format_context::iterator;
};
template<>
struct fmt::formatter<fed::ast::function_name> : indentable {
    constexpr auto parse(fmt::format_parse_context& ctx) {
        return ctx.begin();
    }
    auto format(
        fed::ast::function_name const& exp, 
        fmt::format_context& ctx
    ) const -> fmt::format_context::iterator;
};
template<>
struct fmt::formatter<fed::ast::constant> : indentable {
    constexpr auto parse(fmt::format_parse_context& ctx) {
        return ctx.begin();
    }
    auto format(
        fed::ast::constant const& exp, 
        fmt::format_context& ctx
    ) const -> fmt::format_context::iterator;
};

//
/// TYPE PRETTY PRINT
//

template<>
struct fmt::formatter<fed::ast::type> : indentable {
    constexpr auto parse(fmt::format_parse_context& ctx) {
        return ctx.begin();
    }
    auto format(
        fed::ast::type const& exp, 
        fmt::format_context& ctx
    ) const -> fmt::format_context::iterator;
};

template<>
struct fmt::formatter<fed::ast::type_identifier> : indentable {
    constexpr auto parse(fmt::format_parse_context& ctx) {
        return ctx.begin();
    }
    auto format(
        fed::ast::type_identifier const& exp, 
        fmt::format_context& ctx
    ) const -> fmt::format_context::iterator;
};

template<>
struct fmt::formatter<fed::ast::pointer_type> : indentable {
    constexpr auto parse(fmt::format_parse_context& ctx) {
        return ctx.begin();
    }
    auto format(
        fed::ast::pointer_type const& exp, 
        fmt::format_context& ctx
    ) const -> fmt::format_context::iterator;
};

template<>
struct fmt::formatter<fed::ast::array_type> : indentable {
    constexpr auto parse(fmt::format_parse_context& ctx) {
        return ctx.begin();
    }
    auto format(
        fed::ast::array_type const& exp, 
        fmt::format_context& ctx
    ) const -> fmt::format_context::iterator;
};

template<>
struct fmt::formatter<fed::ast::function_type> : indentable {
    constexpr auto parse(fmt::format_parse_context& ctx) {
        return ctx.begin();
    }
    auto format(
        fed::ast::function_type const& exp, 
        fmt::format_context& ctx
    ) const -> fmt::format_context::iterator;
};

template<>
struct fmt::formatter<fed::ast::argument> : indentable {
    constexpr auto parse(fmt::format_parse_context& ctx) {
        return ctx.begin();
    }
    auto format(
        fed::ast::argument const& exp, 
        fmt::format_context& ctx
    ) const -> fmt::format_context::iterator;
};

template<>
struct fmt::formatter<fed::ast::subrange_type> : indentable {
    constexpr auto parse(fmt::format_parse_context& ctx) {
        return ctx.begin();
    }
    auto format(
        fed::ast::subrange_type const& exp, 
        fmt::format_context& ctx
    ) const -> fmt::format_context::iterator;
};

template<>
struct fmt::formatter<fed::ast::enumerated_type> : indentable {
    constexpr auto parse(fmt::format_parse_context& ctx) {
        return ctx.begin();
    }
    auto format(
        fed::ast::enumerated_type const& exp, 
        fmt::format_context& ctx
    ) const -> fmt::format_context::iterator;
};

template<>
struct fmt::formatter<fed::ast::fixed_field> : indentable {
    constexpr auto parse(fmt::format_parse_context& ctx) {
        return ctx.begin();
    }
    auto format(
        fed::ast::fixed_field const& exp, 
        fmt::format_context& ctx
    ) const -> fmt::format_context::iterator;
};

template<>
struct fmt::formatter<fed::ast::record_type> : indentable {
    constexpr auto parse(fmt::format_parse_context& ctx) {
        return ctx.begin();
    }
    auto format(
        fed::ast::record_type const& exp, 
        fmt::format_context& ctx
    ) const -> fmt::format_context::iterator;
};

template<>
struct fmt::formatter<fed::ast::procedure_type> : indentable {
    constexpr auto parse(fmt::format_parse_context& ctx) {
        return ctx.begin();
    }
    auto format(
        fed::ast::procedure_type const& exp, 
        fmt::format_context& ctx
    ) const -> fmt::format_context::iterator;
};

template<>
struct fmt::formatter<fed::ast::type_builtin> : indentable {
    constexpr auto parse(fmt::format_parse_context& ctx) {
        return ctx.begin();
    }
    auto format(
        fed::ast::type_builtin const& exp, 
        fmt::format_context& ctx
    ) const -> fmt::format_context::iterator;
};

template<>
struct fmt::formatter<fed::ast::set_type> : indentable {
    constexpr auto parse(fmt::format_parse_context& ctx) {
        return ctx.begin();
    }
    auto format(
        fed::ast::set_type const& exp, 
        fmt::format_context& ctx
    ) const -> fmt::format_context::iterator;
};

template<>
struct fmt::formatter<fed::ast::file_type> : indentable {
    constexpr auto parse(fmt::format_parse_context& ctx) {
        return ctx.begin();
    }
    auto format(
        fed::ast::file_type const& exp, 
        fmt::format_context& ctx
    ) const -> fmt::format_context::iterator;
};


//
/// STATEMENTS PRETTY PRINT
//

template<>
struct fmt::formatter<fed::ast::statement> : indentable {
    constexpr auto parse(fmt::format_parse_context& ctx) {
        return ctx.begin();
    }
    auto format(
        fed::ast::statement const& stmt,
        fmt::format_context& ctx
) const -> fmt::format_context::iterator;
};

template<>
struct fmt::formatter<fed::ast::assignment_statement> : indentable {
    constexpr auto parse(fmt::format_parse_context& ctx) {
        return ctx.begin();
    }
    auto format(
        fed::ast::assignment_statement const& stmt,
        fmt::format_context& ctx
) const -> fmt::format_context::iterator;
};

template<>
struct fmt::formatter<fed::ast::procedure_statement> : indentable {
    constexpr auto parse(fmt::format_parse_context& ctx) {
        return ctx.begin();
    }
    auto format(
        fed::ast::procedure_statement const& stmt,
        fmt::format_context& ctx
) const -> fmt::format_context::iterator;
};

template<>
struct fmt::formatter<fed::ast::empty_statement> : indentable {
    constexpr auto parse(fmt::format_parse_context& ctx) {
        return ctx.begin();
    }
    auto format(
        fed::ast::empty_statement const& stmt,
        fmt::format_context& ctx
) const -> fmt::format_context::iterator;
};

template<>
struct fmt::formatter<fed::ast::if_statement> : indentable {
    constexpr auto parse(fmt::format_parse_context& ctx) {
        return ctx.begin();
    }
    auto format(
        fed::ast::if_statement const& stmt,
        fmt::format_context& ctx
) const -> fmt::format_context::iterator;
};

template<>
struct fmt::formatter<fed::ast::case_t> : indentable {
    constexpr auto parse(fmt::format_parse_context& ctx) {
        return ctx.begin();
    }
    auto format(
        fed::ast::case_t const& stmt,
        fmt::format_context& ctx
) const -> fmt::format_context::iterator;
};

template<>
struct fmt::formatter<fed::ast::case_statement> : indentable {
    constexpr auto parse(fmt::format_parse_context& ctx) {
        return ctx.begin();
    }
    auto format(
        fed::ast::case_statement const& stmt,
        fmt::format_context& ctx
) const -> fmt::format_context::iterator;
};

template<>
struct fmt::formatter<fed::ast::with_statement> : indentable {
    constexpr auto parse(fmt::format_parse_context& ctx) {
        return ctx.begin();
    }
    auto format(
        fed::ast::with_statement const& stmt,
        fmt::format_context& ctx
) const -> fmt::format_context::iterator;
};

template<>
struct fmt::formatter<fed::ast::for_statement> : indentable {
    constexpr auto parse(fmt::format_parse_context& ctx) {
        return ctx.begin();
    }
    auto format(
        fed::ast::for_statement const& stmt,
        fmt::format_context& ctx
) const -> fmt::format_context::iterator;
};

template<>
struct fmt::formatter<fed::ast::repeat_statement> : indentable {
    constexpr auto parse(fmt::format_parse_context& ctx) {
        return ctx.begin();
    }
    auto format(
        fed::ast::repeat_statement const& stmt,
        fmt::format_context& ctx
) const -> fmt::format_context::iterator;
};

template<>
struct fmt::formatter<fed::ast::while_statement> : indentable {
    constexpr auto parse(fmt::format_parse_context& ctx) {
        return ctx.begin();
    }
    auto format(
        fed::ast::while_statement const& stmt,
        fmt::format_context& ctx
) const -> fmt::format_context::iterator;
};

template<>
struct fmt::formatter<fed::ast::compound_statement> : indentable {
    constexpr auto parse(fmt::format_parse_context& ctx) {
        return ctx.begin();
    }
    auto format(
        fed::ast::compound_statement const& stmt,
        fmt::format_context& ctx
) const -> fmt::format_context::iterator;
};

template<>
struct fmt::formatter<fed::ast::goto_statement> : indentable {
    constexpr auto parse(fmt::format_parse_context& ctx) {
        return ctx.begin();
    }
    auto format(
        fed::ast::goto_statement const& stmt,
        fmt::format_context& ctx
) const -> fmt::format_context::iterator;
};

#endif
