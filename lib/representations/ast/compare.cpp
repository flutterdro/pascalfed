#include "fed/representations/ast/compare.hpp"
#include "fed/representations/ast/nodes.hpp"
#include <algorithm>

namespace fed::ast {

//
/// TYPES COMPARISONS
//

auto operator==(enumerated_type const& lhs, enumerated_type const& rhs) 
    -> bool {
    return std::ranges::equal(lhs.enum_members, rhs.enum_members);
}
auto operator==(subrange_type const& lhs, subrange_type const& rhs) 
    -> bool {
    return lhs.begin == rhs.begin and lhs.end == rhs.end;
}
auto operator==(type_identifier const& lhs, type_identifier const& rhs) 
    -> bool {
    return lhs.id == rhs.id;
}
auto operator==(array_type const& lhs, array_type const& rhs) 
    -> bool {
    return lhs.component_type == rhs.component_type and
           std::ranges::equal(lhs.index_types, rhs.index_types);
}
auto operator==(fixed_field const& lhs, fixed_field const& rhs)
    -> bool {
    return lhs.type == rhs.type and lhs.name == rhs.name;
}
auto operator==(variant_part const& lhs, variant_part const& rhs)
    -> bool {
    return false;
}
auto operator==(record_type const& lhs, record_type const& rhs)
    -> bool {
    return std::ranges::equal(lhs.fixed_fields, rhs.fixed_fields) and
           lhs.variant_fields == rhs.variant_fields;
}
auto operator==(set_type const& lhs, set_type const& rhs)
    -> bool {
    return lhs.base == rhs.base;
}
auto operator==(file_type const& lhs, file_type const& rhs) 
    -> bool {
    return lhs.component_type == rhs.component_type;
}
auto operator==(function_type const& lhs, function_type const& rhs)
    -> bool {
    return lhs.return_type == rhs.return_type and
           std::ranges::equal(lhs.arguments, rhs.arguments);
}
auto operator==(argument const& lhs, argument const& rhs)
    -> bool {
    return lhs.kind == rhs.kind and
           lhs.name == rhs.name and 
           lhs.type == rhs.type;
}
auto operator==(procedure_type const& lhs, procedure_type const& rhs)
    -> bool {
    return false;
}
auto operator==(pointer_type const& lhs, pointer_type const& rhs)
    -> bool {
    return lhs.base == rhs.base;
}

// 
/// CONSTANTS COMPARISONS
//

auto operator==(enum_constant const& lhs, enum_constant const& rhs)
    -> bool {
    return lhs.ord_value == rhs.ord_value;
}
auto operator==(constant_name const& lhs, constant_name const& rhs)
    -> bool {
    return lhs.id == rhs.id;
} 
auto operator==(integer_literal const& lhs, integer_literal const& rhs)
    -> bool {
    return lhs.value == rhs.value;
}
auto operator==(real_literal const& lhs, real_literal const& rhs)
    -> bool {
    return lhs.value == rhs.value;
}
auto operator==(string_literal const& lhs, string_literal const& rhs)
    -> bool {
    return lhs.value == rhs.value;
}

//
/// EXPRESSION COMPARISONS
//


auto operator==(function_name const& lhs, function_name const& rhs)
    -> bool {
    return lhs.id == rhs.id;
}
auto operator==(variable_name const& lhs, variable_name const& rhs)
    -> bool {
    return lhs.id == rhs.id;
}
auto operator==(indexed_expression const& lhs, indexed_expression const& rhs)
    -> bool {
    return lhs.array == rhs.array and std::ranges::equal(lhs.indecies, rhs.indecies);
}
auto operator==(dereferenced_expression const& lhs, dereferenced_expression const& rhs)
    -> bool {
    return lhs.ptr == rhs.ptr;
}
auto operator==(called_expression const& lhs, called_expression const& rhs)
    -> bool {
    return lhs.callable == rhs.callable and
           std:: ranges::equal(lhs.arguments, rhs.arguments);
}
auto operator==(membered_expression const& lhs, membered_expression const& rhs)
    -> bool {
    return lhs.member == rhs.member and lhs.object == rhs.object;
}
auto operator==(binary_expression const& lhs, binary_expression const& rhs)
    -> bool {
    return lhs.operation == rhs.operation and
           lhs.lhs == rhs.lhs and 
           lhs.rhs == rhs.rhs;
}
auto operator==(unary_expression const& lhs, unary_expression const& rhs)
    -> bool {
    return lhs.operation == rhs.operation and
           lhs.operand == rhs.operand;
}

}
