#ifndef FED_PARSE_TREE_HPP_
#define FED_PARSE_TREE_HPP_

#include "fed/representations/raw-source.hpp"
#include <string_view>
#include <variant>
#include <optional>
#include <vector>


namespace fed {

template<typename T>
using group = std::vector<T>;
using identifier = source::view;
using identifier_group = std::vector<std::string_view>;

struct expression;
struct simple_expression;
struct binary_simple_expression;
struct factor;
struct enumerated_type;
struct term;

struct program_heading {
    source::view region;
    identifier name;
    std::optional<group<identifier>> externals;
};

struct label_declaration {
    group<identifier> labels;
};

// constant is either a number, a constant identifier 
// (possibly signed), a character, or a string
// TODO: handle constant id 
using constant = std::variant<int, double, char, std::string_view>;

struct constant_definition {
    source::view region;
    identifier identifiers;
    constant constants;
};

struct simple_type {};
struct structured_type {};
struct pointer_type {};

using type = std::variant<simple_type, structured_type, pointer_type>;


struct type_definition {
    source::view region;
    identifier name;
    type types;
};

struct variable_declaration {
    source::view region;
    group<identifier> identifiers;
    type type;
};

struct block {
    source::view region;
    std::optional<label_declaration> label_declaration_part;
    std::optional<group<constant_definition>> constant_deginitions;
    std::optional<group<type_definition>> type_definitions;
    std::optional<group<variable_declaration>> variable_declarations;

};

struct formal_parameter_simple {
    bool is_variable;
    group<identifier> names;
    identifier type;
};

struct function_heading;
struct procedure_heading;

using formal_parameter = 
    std::variant<formal_parameter_simple, function_heading, procedure_heading>;

struct function_heading {
    identifier name;
    std::optional<group<formal_parameter>> formal_parametr_list;
    identifier return_type;
};

struct function_forward {
    identifier name;
    group<formal_parameter> formal_parameter_list;
};
struct function_declaration {
    function_heading head;
    std::optional<block> body;
};

struct procedure_heading {
    identifier name;
    std::optional<group<formal_parameter>> formal_parametr_list;
    identifier return_type;
};

struct procedure_forward {
    identifier name;
    group<formal_parameter> formal_parameter_list;
};
struct procedure_implementation {
    procedure_heading head;
    block body;
};

using procedure_declaration = 
    std::variant<procedure_forward, procedure_implementation>;




struct program {
    source::view region;
    program_heading head;
    block           body;
};

struct enumerated_type {
    source::view region;
    identifier_group identifiers;
};


struct expression {
    source::view region;
    std::variant<> ;
};




} // namespace fed



#endif
