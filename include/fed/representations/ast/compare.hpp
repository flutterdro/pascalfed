#ifndef FED_AST_COMP_HPP_ASJNLKNCS
#define FED_AST_COMP_HPP_ASJNLKNCS
#include "fed/representations/ast/forward.hpp"
// this header adds equality comparison between nodes
// it exists just for tests hence seperate header 

// important to note that equality here is dumb comparison between nodes
// say we have next type declarations
// type 
//   intSet = set of Integer;
//   arr1 = array[Integer] of intSet;
//   arr2 = array[Integer] of set of Integer 
// even if arr1 and arr2 semantically the same their types compare false
// because ast is different
// though they would compare true if intSet was an alias since 
// aliases share the same id and ast stores only ids 
//
// as you can see I wrote a moronic system and there is no useful way
// to define proper comparison therefore it is written only with tests 
// in mind 
// it is also the reason why comparison isn't included in ast.hpp 
// it just isn't meant to be used
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
auto operator==(argument const&, argument const&) -> bool;
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

auto operator==(assignment_statement const&, assignment_statement const&) -> bool;
auto operator==(procedure_statement const&, procedure_statement const&) -> bool;
auto operator==(empty_statement const&, empty_statement const&) -> bool;
auto operator==(if_statement const&, if_statement const&) -> bool;
auto operator==(case_t const&, case_t const&) -> bool;
auto operator==(case_statement const&, case_statement const&) -> bool;
auto operator==(with_statement const&, with_statement const&) -> bool;
auto operator==(for_statement const&, for_statement const&) -> bool;
auto operator==(repeat_statement const&, repeat_statement const&) -> bool;
auto operator==(while_statement const&, while_statement const&) -> bool;
auto operator==(compound_statement const&, compound_statement const&) -> bool;
auto operator==(goto_statement const&, goto_statement const&) -> bool;

}
#endif

