#ifndef FED_AST_FORWARD_HPP_HPSOJNC
#define FED_AST_FORWARD_HPP_HPSOJNC 

#include <variant>
#include <string>
#include <string_view>
#include <optional>
#include <vector>

namespace fed::ast {

template<typename T>
class handle;
template<typename T>
class observer_handle;

using std::variant;

using identifier = std::string;
using identifier_view = std::string_view;

template<typename T>
using group = std::vector<T>;
template<typename T>
using maybe = std::optional<T>;

enum class type_id;
enum class variable_id;
enum class constant_id;
enum class function_id;
enum class procedure_id;

struct type_declaration;
struct variable_declaration;
struct constant_declaration;
struct function_declaration;
struct procedure_declaration;

struct argument;
enum class argument_kind;

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
enum class type_builtin;

using type = variant<
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

struct fixed_field;
struct variant_field;
struct variant_part;
using fixed_part = group<handle<fixed_field>>;


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

struct function_name;
struct variable_name;

using expression_atom = variant<
    constant,
    function_name,
    variable_name
>;

struct indexed_expression;
struct dereferenced_expression;
struct called_expression;
struct membered_expression;

using expression_leaf = variant<
    expression_atom,
    indexed_expression,
    dereferenced_expression,
    called_expression,
    membered_expression
>;
struct binary_expression;
struct unary_expression;

using expression = variant<
    binary_expression,
    unary_expression,
    expression_leaf
>;

} // namespace fed

#endif
