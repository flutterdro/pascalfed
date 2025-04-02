#ifndef FED_PARSE_TREE_HPP_
#define FED_PARSE_TREE_HPP_

#include "fed/diagnostics/internal-error.hpp"
#include "fed/representations/raw-source.hpp"
#include "fed/representations/ast/handle.hpp"
#include "fed/representations/ast/forward.hpp"
#include "fed/representations/symbol-table.hpp"
#include "fed/utils/superutil.hpp"

#include <cstddef>
#include <fmt/base.h>
#include <fmt/chrono.h>
#include <fmt/format.h>

#include <functional>
#include <memory>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <variant>
#include <optional>
#include <vector>

namespace fed {
}
namespace fed::ast {






struct enumerated_type;

struct type_declaration;
using type_id = symbol_mapback<handle<type_declaration>>::id;
struct type_identifier {
    type_id id;
    handle<identifier> identifier;
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
using constant_id = symbol_mapback<ast::handle<constant_declaration>>::id;

struct enumerated_type;
struct subrange_type;
struct type_identifier;
struct array_type;
struct record_type;
struct set_type;
struct file_type;
struct function_type;
struct procedure_type;
struct pointer_type;
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
    group<identifier> enum_members;
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

struct function_type {
    handle<type> return_type;
    group<handle<type>> argument_types;
};

struct procedure_type {
    group<handle<type>> argument_types;
};

struct fixed_fields {
    group<handle<type>>             member_types;
    name_map<observer_handle<type>> members;
};
struct variant_part {
    group<handle<constant>> matches;
    handle<record_type> fields;
};
struct variant_field {
    handle<identifier> name;
    handle<type> tag;
    group<handle<variant_part>> variants;
};

struct record_type {
    fixed_fields fixed_part; 
    std::optional<handle<variant_field>> variant_part;
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

using variable_declaration_handle = handle<variable_declaration>;

struct block {
    source::view region;
    std::optional<label_declaration> label_declaration_part;
    std::optional<group<constant_declaration>> constant_deginitions;
    std::optional<group<type_declaration>> type_definitions;
    std::optional<group<variable_declaration>> variable_declarations;

};

struct formal_parameter_simple {
    bool is_variable;
    group<handle<identifier>> names;
    handle<identifier> type;
};

struct function_heading;
struct procedure_heading;

using formal_parameter = 
    std::variant<formal_parameter_simple, function_heading, procedure_heading>;



struct function_heading {
    handle<identifier> name;
    std::optional<group<handle<formal_parameter>>> formal_parametr_list;
    handle<type> return_type;
};

struct function_declaration {
    handle<type> type;
    handle<block> body;
};
using function_declaration_handle = handle<function_declaration>;

struct procedure_heading {
    handle<identifier> name;
    std::optional<group<handle<formal_parameter>>> formal_parametr_list;
    handle<identifier> return_type;
};


struct procedure_declaration {
    handle<procedure_heading> head;
    std::optional<handle<block>> body;
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


using function_id = symbol_mapback<ast::handle<function_declaration>>::id;
using variable_id = symbol_mapback<ast::handle<variable_declaration>>::id;
using constant_id = symbol_mapback<ast::handle<constant_declaration>>::id;
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






} // namespace fed

#endif
