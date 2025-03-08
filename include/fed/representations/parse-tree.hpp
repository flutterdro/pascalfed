#ifndef FED_PARSE_TREE_HPP_
#define FED_PARSE_TREE_HPP_

#include "fed/diagnostics/internal-error.hpp"
#include "fed/representations/raw-source.hpp"
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
struct poison_t{};
inline constexpr poison_t poison_pill;
}
namespace fed::ast {

template<typename T>
class handle {
    template<typename>
    friend struct observer_handle;
public:
    handle() 
        : m_handle(nullptr) {}
    handle(T&& val)
        : m_handle(std::make_unique<T>(std::move(val))) {}
    handle(handle const&) = delete;
    handle(handle&&) noexcept = default;
    template<typename U>
    handle(handle<U>&& derived)
        : m_handle(std::move(derived)) {}
    auto operator=(T&& val)
        -> handle& {
        m_handle = std::make_unique<T>(std::move(val));

        return *this;
    }
    constexpr handle(poison_t) noexcept
        :m_handle(nullptr) {}
    auto operator=(handle const&) = delete;
    auto operator=(handle&&) -> handle& = default;
    template<typename U>
    auto operator=(handle<U>&& derived)
        -> handle& {
        *this = std::move(*derived.m_handle);

        return *this;
    }

    auto operator*()
        -> T& { 
        if (is_poisoned()) throw internal_error("accessing a poisoined handle");
        return *m_handle;
    }
    auto operator*() const 
        -> T const& { 
        if (is_poisoned()) throw internal_error("accessing a poisoined handle");
        return *m_handle;
    }
    auto operator->() 
        -> T* { 
        if (is_poisoned()) throw internal_error("accessing a poisoined handle");
        return m_handle.get(); 
    }
    auto operator->() const
        -> T const* { 
        if (is_poisoned()) throw internal_error("accessing a poisoined handle");
        return m_handle.get(); 
    }
    
    auto get()
        -> T* { 
        if (is_poisoned()) throw internal_error("accessing a poisoined handle");
        return m_handle.get(); 
    }
    auto get() const
        -> T const* { 
        if (is_poisoned()) throw internal_error("accessing a poisoined handle");
        return m_handle.get(); 
    }

    auto poison()
        -> void { m_handle = nullptr; }
    auto is_poisoned() const
        -> bool { return m_handle == nullptr; }
private:
    std::unique_ptr<T> m_handle{};

};
// non - owning, immutable handle
template<typename T>
class observer_handle {
    friend handle<T>;
public:
    observer_handle() = delete;
    constexpr observer_handle(poison_t) noexcept 
        : m_handle(nullptr) {}
    
    observer_handle(T const* ptr)
        : m_handle(ptr) {
        if (ptr == nullptr) throw internal_error("observer_handle cannot be constructed from nullptr");
    }
    observer_handle(nullptr_t) = delete;
    observer_handle(handle<T> const& handle)
        : m_handle(handle.m_handle.get()) {}
    observer_handle(observer_handle const&) = default;
    auto is_poisoned() const noexcept
        -> bool { return m_handle == nullptr; }
    auto and_then(auto&& f) const noexcept {
        if (is_poisoned()) 
            return std::remove_cvref_t<std::invoke_result_t<decltype(f), T const&>>(poison_pill);
        return std::invoke(FWD(f), *this->m_handle);
    }

    auto unsafe_get() const noexcept { return m_handle; }

private:
    T const* m_handle;
};

template<typename F, typename... Ts>
auto then_all(F&& func, observer_handle<Ts>... handles)
    -> std::invoke_result_t<F, Ts const&...> {
    if ((handles.is_poisoned() or ...)) 
        return std::invoke_result_t<F, Ts const&...>(poison_pill);
    return std::invoke(FWD(func), *handles.unsafe_get()...);
}


using identifier = std::string;
using identifier_view = std::string_view;


template<typename T>
using group = std::vector<T>;
template<typename... Ts>
using variant = std::variant<Ts...>;
template<typename T>
using maybe = std::optional<T>;

namespace detail {
struct string_hash
{
    using hash_type = std::hash<std::string_view>;
    using is_transparent = void;
 
    std::size_t operator()(const char* str) const        { return hash_type{}(str); }
    std::size_t operator()(std::string_view str) const   { return hash_type{}(str); }
    std::size_t operator()(std::string const& str) const { return hash_type{}(str); }
};
} // namespace detail 
template<typename T>
using name_map = std::unordered_map<std::string, T, detail::string_hash, std::equal_to<>>;


struct enumerated_type;

struct type_declaration;
using type_id = symbol_mapback<type_declaration>::id;
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
using constant = std::variant<int, double, char, std::string_view>;

struct constant_declaration {
    source::view region;
    handle<identifier> identifiers;
    handle<constant> constants;
};
using constant_declaration_handle = handle<constant_declaration>;

struct simple_type {};
struct structured_type {};
struct pointer_type {};

struct enumerated_type;
struct subrange_type;
struct type_identifier;
struct array_type;
struct record_type;
struct set_type;
struct file_type;
struct function_type;
struct procedure_type;

using type = std::variant<
    enumerated_type,
    subrange_type,
    type_identifier,
    array_type,
    record_type,
    set_type,
    file_type,
    function_type,
    procedure_type
>;

struct enum_member {};
struct enumerated_type {
    source::view region;
    group<handle<identifier>> identifiers;
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
    handle<identifier> name;
    handle<type> types;
};
using type_declaration_handle = handle<type_declaration>;
struct variable_declaration {
    source::view region;
    group<handle<identifier>> identifiers;
    handle<type> type;
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
    handle<function_heading> head;
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

struct function_name;
struct variable_name;
struct constant_name;
struct enum_name;
struct string_literal;
struct number_literal;

using bare_name = variant<
    function_name,
    variable_name,
    constant_name,
    string_literal,
    number_literal,
    enum_name
>;

struct indexed_variable;
struct dereferenced_variable;
struct called_variable;
struct membered_variable;

using expression_leaf = variant<
    bare_name,
    indexed_variable,
    dereferenced_variable,
    called_variable,
    membered_variable
>;
struct binary_expression;
struct unary_expression;

using expression = variant<binary_expression, unary_expression, expression_leaf>;

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


using function_id = symbol_mapback<ast::observer_handle<function_declaration>>::id;
using variable_id = symbol_mapback<ast::observer_handle<variable_declaration>>::id;
using constant_id = symbol_mapback<ast::observer_handle<constant_declaration>>::id;
using enum_id = symbol_mapback<ast::observer_handle<enumerated_type>>::id;
template<typename IdT>
struct name_from_id;
template<>
struct name_from_id<function_id> { using type = function_name; };
template<>
struct name_from_id<variable_id> { using type = variable_name; };
template<>
struct name_from_id<constant_id> { using type = constant_name; };
template<>
struct name_from_id<enum_id> { using type = enum_name; };
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
struct constant_name {
    observer_handle<type> type;
    constant_id id;
};
struct enum_name {
    observer_handle<type> type;
    enum_id id;
};
struct number_literal {
    observer_handle<type> type;
    unsigned num;
};
struct string_literal {
    observer_handle<type> type;
    source::view string;
};

struct indexed_variable {
    observer_handle<type> type;
    handle<expression> array;
    group<handle<expression>> indecies;
};
struct called_variable {
    observer_handle<type> type;
    handle<expression> callable;
    group<handle<expression>> arguments;
};
struct dereferenced_variable {
    observer_handle<type> type;
    handle<expression> ptr;
};
struct membered_variable {
    observer_handle<type> type;
    handle<expression>    object;
    identifier            member;
};



} // namespace fed

#endif
