#ifndef FED_PARSE_TREE_HPP_
#define FED_PARSE_TREE_HPP_

#include "fed/diagnostics/internal-error.hpp"
#include "fed/representations/raw-source.hpp"
#include "fed/representations/symbol-table.hpp"
#include "fed/utils/superutil.hpp"

#include <fmt/base.h>
#include <fmt/format.h>

#include <memory>
#include <string_view>
#include <utility>
#include <variant>
#include <optional>
#include <vector>


namespace fed::ast {

template<typename T>
class handle {
public:
    handle() 
        : m_handle(nullptr) {}
    handle(T&& val)
        : m_handle(std::make_unique<T>(std::move(val))) {}
    handle(handle const&) = delete;
    handle(handle&&) = default;
    template<typename U>
    handle(handle<U>&& derived)
        : m_handle(std::move(derived)) {}
    auto operator=(T&& val)
        -> handle& {
        m_handle = std::make_unique<T>(std::move(val));

        return *this;
    }
    template<typename U>
    friend class handle;
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
        if (m_handle == nullptr) throw internal_error("accessing empty handle");
        return *m_handle;
    }
    auto operator*() const 
        -> T const& { 
        if (is_poisoned()) throw internal_error("accessing a poisoined handle");
        if (m_handle == nullptr) throw internal_error("accessing empty handle");
        return *m_handle;
    }
    auto operator->() 
        -> T* { 
        if (is_poisoned()) throw internal_error("accessing a poisoined handle");
        if (m_handle == nullptr) throw internal_error("accessing empty handle");
        return m_handle.get(); 
    }
    auto operator->() const
        -> T const* { 
        if (is_poisoned()) throw internal_error("accessing a poisoined handle");
        if (m_handle == nullptr) throw internal_error("accessing empty handle");
        return m_handle.get(); 
    }
    
    auto get()
        -> T* { return m_handle.get(); }
    auto get() const
        -> T const* { return m_handle.get(); }

    auto poison()
        -> void { m_is_poisoned = true; }
    auto is_poisoned() const
        -> bool { return m_is_poisoned; }
private:
    std::unique_ptr<T> m_handle{};
    bool m_is_poisoned{};

};

template<typename... Ts>
class variant_handle {
public:
    template<typename T>
    friend class handle;
    constexpr variant_handle() = default;
    constexpr variant_handle(variant_handle&&) noexcept = default;
    constexpr auto operator=(variant_handle&&) noexcept
        -> variant_handle& = default;
    template<typename T>
    constexpr variant_handle(handle<T> val) noexcept
        : m_variant(std::move(val)) {}
    template<typename T>
    constexpr auto operator=(handle<T> val) noexcept
        -> variant_handle& { 
        m_variant = std::move(val);
        return *this;
    }

    template<typename T>
    friend constexpr auto holds_alternative(variant_handle const& variant) noexcept
        -> bool { return std::holds_alternative<handle<T>>(variant.m_variant); }
    template<typename T>
    friend constexpr auto get(auto&& variant) noexcept 
        -> decltype(auto) { return std::get<T>(FWD(variant)); }
    friend constexpr auto visit(auto&& visitor, auto&&... variants)
        -> decltype(auto) { 
        return std::visit(
            func::psie_combinator(
                FWD(visitor), 
                func::deep_dereference
            ), FWD(variants).m_variant...
        ); 
    }

    constexpr auto is_poisoned() const noexcept
        -> bool { 
        return std::visit(
            [](auto const& val) { return val.is_poisoned(); },
            m_variant
        );
    }
private:
    std::variant<handle<Ts>...> m_variant;
};
template<typename... Ts>
class handle<variant_handle<Ts...>> {
public:
    using value_type = variant_handle<Ts...>;
    handle() = default;
    handle(value_type&& val)
        : m_handle(std::move(val)) {}
    handle(handle const&) = delete;
    handle(handle&&) noexcept = default;
    template<typename U>
    handle(handle<U>&& derived)
        : m_handle(std::move(derived)) {}
    auto operator=(value_type&& val)
        -> handle& {
        m_handle = std::move(val);

        return *this;
    }
    template<typename U>
    friend class handle;
    auto operator=(handle const&) = delete;
    auto operator=(handle&&) noexcept -> handle& = default;
    template<typename U>
    auto operator=(handle<U>&& derived)
        -> handle& {
        *this = std::move(derived);

        return *this;
    }

    auto operator*()
        -> value_type& {
        if (is_poisoned()) throw internal_error("accessing a poisoined handle");
        return m_handle;
    }
    auto operator*() const 
        -> value_type const& { 
        if (is_poisoned()) throw internal_error("accessing a poisoined handle");
        return m_handle;
    }
    auto is_poisoned() const
        -> bool { return m_handle.is_poisoned(); }
private:
    variant_handle<Ts...> m_handle{};
    bool m_is_poisoned{};

};
template<typename T>
using group = std::vector<T>;

using identifier_group = std::vector<std::string_view>;

struct enumerated_type;

struct identifier {
    source::view view;
};
struct type_identifier {
    sym::type::id id;
    handle<identifier> identifier;
};
struct variable_identifier {
    sym::variable::id id;
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

struct constant_definition {
    source::view region;
    handle<identifier> identifiers;
    handle<constant> constants;
};

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

using type = std::variant<
    enumerated_type,
    subrange_type,
    type_identifier,
    array_type,
    record_type,
    set_type,
    file_type
>;





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


struct fixed_field {
    group<handle<identifier>> names;
    handle<type> type;
};
struct variant {
    group<handle<constant>> matches;
    handle<record_type> fields;
};
struct variant_field {
    handle<identifier> name;
    handle<type> tag;
    group<handle<variant>> variants;
};

struct record_type {
    group<handle<fixed_field>> fixed_fields; 
    std::optional<handle<variant_field>> variant_part;
};

struct type_definition {
    source::view region;
    handle<identifier> name;
    handle<type> types;
};

struct variable_declaration {
    source::view region;
    group<handle<identifier>> identifiers;
    handle<type> type;
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
    std::optional<handle<block>> body;
};

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

struct binary_expression;
struct unary_expression;
struct expression_leaf;

using expression = variant_handle<binary_expression, unary_expression, expression_leaf>;

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
    handle<expression> lhs;
    handle<expression> rhs;
    binary_operation operation;
};

struct expression_leaf {
    char character;
};

struct unary_expression {
    handle<expression> operand;
    unary_operation operation;
};







} // namespace fed

template<>
struct fmt::formatter<fed::ast::binary_operation> {
    constexpr auto parse(fmt::format_parse_context& ctx) 
        -> fmt::format_parse_context::iterator { return ctx.begin(); }
    constexpr auto format(fed::ast::binary_operation const& op, fmt::format_context& ctx) const 
        -> fmt::format_context::iterator {
        switch (op) {

        case fed::ast::binary_operation::add: *ctx.out()++ = '+';break;
        case fed::ast::binary_operation::substract:*ctx.out()++ = '-';break;
        case fed::ast::binary_operation::or_:*ctx.out()++ = 'o';break;
        case fed::ast::binary_operation::multiply:*ctx.out()++ = '*';break;
        case fed::ast::binary_operation::integer_divide:*ctx.out()++ = '/';break;
        case fed::ast::binary_operation::real_divide:*ctx.out()++ = '/';break;
        case fed::ast::binary_operation::modulo:*ctx.out()++ = '%';break;
        case fed::ast::binary_operation::and_:*ctx.out()++ = 'a';break;
        case fed::ast::binary_operation::equal:*ctx.out()++ = '=';break;
        case fed::ast::binary_operation::not_equal:*ctx.out()++ = '=';break;
        case fed::ast::binary_operation::greater:*ctx.out()++ = '>';break;
        case fed::ast::binary_operation::less:*ctx.out()++ = '<';break;
        case fed::ast::binary_operation::greater_or_equal:*ctx.out()++ = '>';break;
        case fed::ast::binary_operation::less_or_equal:*ctx.out()++ = '<';break;
        case fed::ast::binary_operation::in:*ctx.out()++ = 'i';break;
          break;
        }
        return ctx.out();
    }
};
template<>
struct fmt::formatter<fed::ast::expression> {
    int depth = 0;
    constexpr formatter() = default;
    constexpr formatter(int depth_) 
        : depth(depth_) {}
    constexpr auto parse(fmt::format_parse_context& ctx) 
        -> fmt::format_parse_context::iterator { return ctx.begin(); }
    constexpr auto format(fed::ast::expression const& expr, fmt::format_context& ctx) const 
        -> fmt::format_context::iterator {
        int a = 0;
        ctx.out() = fmt::format_to(ctx.out(), "{0:>{1}}", "", 2*depth);
        visit(
        fed::overloaded{
            [&ctx, this](fed::ast::binary_expression const& exp) {
                    ctx.out() = fmt::format_to(ctx.out(), "|binary expression {}\n", exp.operation);
                    ctx.out() = fmt::formatter<fed::ast::expression>(this->depth + 1).format(*exp.lhs, ctx);
                    ctx.out() = fmt::formatter<fed::ast::expression>(this->depth + 1).format(*exp.rhs, ctx);
                },
            [&ctx, this](fed::ast::unary_expression const& exp) {
                    ctx.out() = fmt::format_to(ctx.out(), "|unary expression\n");
                    ctx.out() = fmt::formatter<fed::ast::expression>(this->depth + 1).format(*exp.operand, ctx);
                },
            [&ctx, this](fed::ast::expression_leaf const& exp) {
                    ctx.out() = fmt::format_to(ctx.out(), "|leaf letter {}\n", exp.character);
                }
        }, expr);
        return ctx.out();
    }
};


#endif
