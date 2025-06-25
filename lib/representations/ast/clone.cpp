#include "fed/representations/ast/clone.hpp"
#include "fed/representations/ast/forward.hpp"
#include "fed/representations/ast/nodes.hpp"

namespace fed::ast {


auto clone(identifier const& val) -> identifier {
    return val;
}

auto clone(enumerated_type const& val) -> enumerated_type {
    return {.enum_members = ast::clone(val.enum_members)};
}

auto clone(subrange_type const& val) -> subrange_type {
    return {.begin = ast::clone(val.begin), .end = ast::clone(val.end)};
}
auto clone(type_identifier const& val) -> type_identifier {
    return val;
}
auto clone(array_type const& val) -> array_type {
    return {
        .index_types    = ast::clone(val.index_types),
        .component_type = ast::clone(val.component_type),
    };
}
auto clone(fixed_field const& val) -> fixed_field {
    return {
        .name = ast::clone(val.name),
        .type = ast::clone(val.type)
    };
}
auto clone(record_type const& val) -> record_type {
    return {
        .fixed_fields = ast::clone(val.fixed_fields)
    };
}
auto clone(set_type const& val) -> set_type {
    return {
        .base = ast::clone(val.base),
    };
}
auto clone(file_type const& val) -> file_type {
    return {
        .component_type = ast::clone(val.component_type)
    };
}
auto clone(argument const& val) -> argument {
    return {
        .name = val.name,
        .type = ast::clone(val.type),
        .kind = val.kind,
    };
}
auto clone(function_type const& val) -> function_type {
    return {
        .return_type = ast::clone(val.return_type),
        .arguments = ast::clone(val.arguments),
    };
}
auto clone(procedure_type const& val) -> procedure_type {
    return {
        .argument_types = ast::clone(val.argument_types),
    };
}
auto clone(pointer_type const& val) -> pointer_type {
    return {
        .base = ast::clone(val.base),
    };
}
auto clone(type const& val) -> type {
    return std::visit(chain| LIFT(clone)| construct<type>, val);
}
auto clone(enum_constant const& val) -> enum_constant {
    return {.type = val.type, .ord_value = val.ord_value};
}
auto clone(constant_name const& val) -> constant_name {
    return {.type = val.type, .id = val.id};
}
auto clone(integer_literal const& val) -> integer_literal {
    return {.value = val.value};
}
auto clone(real_literal const& val) -> real_literal {
    return {.value = val.value};
}
auto clone(string_literal const& val) -> string_literal {
    return {.value = val.value};
}
auto clone(constant const& val) -> constant {
    return std::visit(chain| LIFT(clone)| construct<constant>, val);
}


}





