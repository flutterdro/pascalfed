#ifndef FED_AST_COMP_HPP_ASJNLKNCS
#define FED_AST_COMP_HPP_ASJNLKNCS
#include "fed/representations/ast/forward.hpp"
// this header adds equality comparison between nodes
// it exists just for tests hence seperate header 
namespace fed::ast{

auto operator==(enumerated_type const&, enumerated_type const&) -> bool;
auto operator==(subrange_type const&, subrange_type const&) -> bool;
auto operator==(type_identifier const&, type_identifier const&) -> bool;
auto operator==(array_type const&, array_type const&) -> bool;
auto operator==(record_type const&, record_type const&) -> bool;
auto operator==(set_type const&, set_type const&) -> bool;
auto operator==(file_type const&, file_type const&) -> bool;
auto operator==(function_type const&, function_type const&) -> bool;
auto operator==(procedure_type const&, procedure_type const&) -> bool;
auto operator==(pointer_type const&, pointer_type const&) -> bool;

auto operator==(enum_constant const&, enum_constant const&) -> bool;
auto operator==(constant_name const&, constant_name const&) -> bool; 
auto operator==(integer_literal const&, integer_literal const&) -> bool;
auto operator==(real_literal const&, real_literal const&) -> bool;
auto operator==(string_literal const&, string_literal const&) -> bool;

auto operator==(function_name const&, function_name const&) -> bool;
auto operator==(variable_name const&, variable_name const&) -> bool;
auto operator==(indexed_expression const&, indexed_expression const&) -> bool;
auto operator==(dereferenced_expression const&, dereferenced_expression const&) -> bool;
auto operator==(called_expression const&, called_expression const&) -> bool;
auto operator==(membered_expression const&, membered_expression const&) -> bool;
auto operator==(binary_expression const&, binary_expression const&) -> bool;
auto operator==(unary_expression const&, unary_expression const&) -> bool;


}
#endif

