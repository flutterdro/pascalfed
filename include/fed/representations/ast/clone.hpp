#ifndef FED_AST_CLONNING_HPP_
#define FED_AST_CLONNING_HPP_

#include "fed/representations/ast/forward.hpp"
#include "fed/utils/superutil.hpp"
#include <iterator>

namespace fed::ast {



auto clone(identifier const& val) -> identifier;

auto clone(enumerated_type const&) -> enumerated_type;
auto clone(subrange_type const&) -> subrange_type;
auto clone(type_identifier const&) -> type_identifier;
auto clone(array_type const&) -> array_type;
auto clone(record_type const&) -> record_type;
auto clone(fixed_field const&) -> fixed_field;
auto clone(set_type const&) -> set_type;
auto clone(file_type const&) -> file_type;
auto clone(function_type const&) -> function_type;
auto clone(argument const&) -> argument;
auto clone(procedure_type const&) -> procedure_type;
auto clone(pointer_type const&) -> pointer_type;
auto clone(type const&) -> type;

auto clone(enum_constant const&) -> enum_constant;
auto clone(constant_name const&) -> constant_name;
auto clone(integer_literal const&) -> integer_literal;
auto clone(real_literal const&) -> real_literal;
auto clone(string_literal const&) -> string_literal;
auto clone(constant const&) -> constant;

template<typename T>
auto clone(handle<T> const& val) -> handle<T> {
    return val.and_then(
        chain| LIFT(clone) | construct<handle<T>>
    );
}
template<typename T>
auto clone(group<T> const& val) -> group<T> {
    auto result = group<T>();
    std::ranges::transform(val, std::back_inserter(result), LIFT(clone));
    return result;
}
template<typename T>
auto clone(maybe<T> const& val) -> maybe<T> {
    return val.transform(LIFT(clone));
}



}


#endif
