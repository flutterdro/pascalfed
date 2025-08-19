#ifndef FED_PARSE_TREE_HPP_
#define FED_PARSE_TREE_HPP_

#include "fed/parser/context.hpp"
#include "fed/representations/raw-source.hpp"
#include "fed/representations/ast/handle.hpp"
#include "fed/representations/ast/forward.hpp"

#include <cstddef>
#include <fmt/base.h>
#include <fmt/chrono.h>
#include <fmt/format.h>

#include <variant>
#include <optional>

namespace fed {
}
namespace fed::ast {

struct block {
    semantic_context context;
    group<function>  functions;
    group<procedure> procedures;
};

struct function {
    function_id   id;
    handle<block> block;
};
struct procedure {
    procedure_id  id;
    handle<block> block;
};

enum class type_id      { poison = 0, };
enum class variable_id  { poison = 0, };
enum class constant_id  { poison = 0, };
enum class function_id  { poison = 0, };
enum class procedure_id { poison = 0, };

struct type_identifier {
    type_id id;
};

struct program_heading {
    source::view region;
    handle<identifier> name;
    std::optional<group<handle<identifier>>> externals;
};

struct label_declaration {
    group<handle<identifier>> labels;
};

// constant is either a number, a constant identifier 
// (possibly signed), a character, or a string
// TODO: handle constant id 
struct enum_constant;
struct constant_name; 
struct integer_literal;
struct real_literal;
struct string_literal;
using constant = variant<
    constant_name,
    integer_literal, 
    real_literal,
    enum_constant,
    string_literal
>;

struct constant_declaration {
    identifier name;
    handle<constant> constant;
};

enum class type_builtin { integer = 1, boolean, real, character, };

using type = std::variant<
    enumerated_type,
    subrange_type,
    type_identifier,
    array_type,
    record_type,
    set_type,
    file_type,
    function_type,
    pointer_type,
    procedure_type,
    type_builtin
>;

struct enum_member {};
struct enumerated_type {
    group<handle<identifier>> enum_members;
};

struct pointer_type {
    handle<type> base;
};

struct subrange_type {
    handle<constant> begin;
    handle<constant> end;
};

struct array_type {
    group<handle<type>> index_types;
    handle<type> component_type;
};

struct set_type {
    handle<type> base;
};

struct file_type {
    handle<type> component_type;
};
enum class argument_kind {
    copy,
    ref,
};
struct argument {
    maybe<identifier> name;
    handle<type>      type;
    argument_kind     kind;
    
};
struct function_type {
    handle<type> return_type;
    group<handle<argument>> arguments;
};

struct procedure_type {
    group<handle<type>> argument_types;
};

struct fixed_field  {
    identifier name;
    handle<type> type;
};
struct variant_field {
    handle<constant>   name;
    handle<fixed_part> fields;
};
struct variant_part {
    maybe<identifier>            name;
    type_identifier              discrimination_type;
    group<handle<variant_field>> fields;
};

using fixed_part = group<handle<fixed_field>>;
struct record_type {
    fixed_part   fixed_fields;
    maybe<handle<variant_part>> variant_fields;
};

struct type_declaration {
    source::view region;
    identifier name;
    handle<type> type;
};
struct variable_declaration {
    identifier name;
    handle<type> type;
};

struct enum_constant {
    observer_handle<type> type;
    int ord_value;
};
struct function_constant {
    observer_handle<type> type;
    function_id id;
};
struct procedure_constant {
    observer_handle<type> type;
    procedure_id id;
};
struct constant_name {
    observer_handle<type> type;
    constant_id id;
};
struct integer_literal {
    int value;
};
struct real_literal {
    double value;
};
struct string_literal {
    std::string value;
};





struct function_heading;
struct procedure_heading;

struct function_declaration {
    identifier   name;
    handle<type> type;
    handle<block> body;
};

struct procedure_declaration {
    identifier name;
};

struct program {
    source::view region;
    handle<program_heading> head;
    handle<block>           body;
};


//
/// EXPRESSION AST
//

enum class binary_operation {
    add,
    substract,
    or_,

    multiply,
    integer_divide,
    real_divide,
    modulo,
    and_,

    equal,
    not_equal,
    greater,
    less,
    greater_or_equal,
    less_or_equal,
    in,
};

enum class unary_operation {
    logical_negation,

    negation,
    identity,
};

struct binary_expression {
    observer_handle<type> type;
    handle<expression> lhs;
    handle<expression> rhs;
    binary_operation operation;
};

struct unary_expression {
    observer_handle<type> type;
    handle<expression> operand;
    unary_operation operation;
};


template<typename IdT>
struct name_from_id;
template<>
struct name_from_id<function_id> { using type = function_name; };
template<>
struct name_from_id<variable_id> { using type = variable_name; };
template<>
struct name_from_id<constant_id> { using type = constant_name; };
template<typename IdT>
using name_from_id_t = name_from_id<IdT>::type;
struct function_name {
    observer_handle<type> type;
    function_id id;
};
struct variable_name {
    observer_handle<type> type;
    variable_id id;
};
struct number_literal {
    observer_handle<type> type;
    unsigned num;
};

struct indexed_expression {
    observer_handle<type> type;
    handle<expression> array;
    group<handle<expression>> indecies;
};
struct called_expression {
    observer_handle<type> type;
    handle<expression> callable;
    group<handle<expression>> arguments;
};
struct dereferenced_expression {
    observer_handle<type> type;
    handle<expression> ptr;
};
struct membered_expression {
    observer_handle<type> type;
    handle<expression>    object;
    identifier            member;
};


//
/// STATEMENTS
//
struct empty_statement {};

struct assignment_statement {
    variable_id        variable;
    handle<expression> value;
};

struct procedure_statement {
    procedure_id             procedure;
    handle_group<expression> call_args;
};

struct if_statement {
    handle<expression>        condition;
    handle<statement>         then_case;
    maybe<handle<statement>>  else_case;
};
struct case_t {
    handle_group<constant> values;
    handle<statement>      action;  
};
struct case_statement {
    handle<expression>   case_index;
    handle_group<case_t> cases;
};

struct while_statement {
    handle<expression> condition;
    handle<statement>  action;
};

struct repeat_statement {
    handle<expression> condition;
    handle<statement>  action;
};

struct for_statement {
    semantic_context   ctx;
    variable_id        control_variable;
    enum class iteration_t {
        ascending, descending, bad
    } iteration;
    handle<expression> initial_value;
    handle<expression> final_value;
    handle<statement>  action;
};

struct with_statement {
    semantic_context   ctx;
    group<variable_id> variables;
    handle<statement>  action;
};

struct compound_statement {
    handle_group<statement> statements;
};

struct goto_statement {};





} // namespace fed

#endif
