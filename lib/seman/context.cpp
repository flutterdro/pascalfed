#include "fed/seman/context.hpp"
#include "fed/diagnostics/compile-error.hpp"
#include "fed/representations/ast.hpp"
#include "fed/representations/ast/forward.hpp"
#include "fed/representations/ast/handle.hpp"
#include "fed/representations/ast/name-scope.hpp"
#include "fed/representations/ast/nodes.hpp"
#include "fed/representations/ast/sym-table.hpp"
#include "fed/utils/predicates.hpp"
#include "fed/utils/superutil.hpp"
#include <ranges>
#include <functional>
#include <limits>
#include <memory>
#include <optional>
#include <span>
#include <string_view>
#include <utility>


namespace fed {
constexpr auto const integer_min = std::numeric_limits<int>::min();
constexpr auto const integer_max = std::numeric_limits<int>::max();

semantic_context::semantic_context(
    ast::symbol_table* table, 
    std::unique_ptr<ast::name_scope> names, 
    bool is_global
)   : m_table_ptr(table)
    , m_names_ptr(std::move(names))
    , m_is_global(is_global) {}
semantic_context::semantic_context(semantic_context&& ctx) noexcept
    : m_table_ptr(ctx.m_table_ptr)
    , m_names_ptr(std::move(ctx.m_names_ptr))
    , m_is_global(ctx.m_is_global) {
    ctx.m_table_ptr = nullptr;
    ctx.m_is_global = false;
}
auto semantic_context::operator=(semantic_context&& ctx) noexcept
    -> semantic_context& {
    m_table_ptr = ctx.m_table_ptr;
    m_names_ptr = std::move(ctx.m_names_ptr);
    m_is_global = ctx.m_is_global;
    ctx.m_table_ptr = nullptr;
    ctx.m_is_global = false;

    return *this;
}
semantic_context::~semantic_context() noexcept {
    if (m_is_global) {
        delete m_table_ptr;
    }
}

auto semantic_context::make_global()
    -> semantic_context {
    auto ctx = semantic_context(
        new ast::symbol_table(),
        std::make_unique<ast::name_scope>(),
        true
    );
    ctx.init_poison_swamp();
    ctx.init_builtin_types();
    // ctx.init_builtin_constants();

    return ctx;
}
auto semantic_context::make_local() const noexcept
    -> semantic_context {
    return semantic_context(
        m_table_ptr,
        names().spawn(),
        false
    );
}

auto semantic_context::init_poison_swamp()
    -> void {
    add_type(ast::type_declaration{ // NOLINT
        .name = "##Poisoned Type",
        .type = poison_pill,
    });
    add_constant(ast::constant_declaration{ // NOLINT
        .name = "##Poisoned Constant",
        .constant = poison_pill,
    });
    add_variable(ast::variable_declaration{ // NOLINT
        .name = "##Poisoned Variable",
        .type = poison_pill,
    });
}
auto semantic_context::init_builtin_types()
    -> void {
    add_type( // NOLINT
        ast::type_declaration{
            .name = ast::identifier("Integer"),
            .type = ast::type(ast::type_builtin::integer),
        }
    );
    add_type( // NOLINT
        ast::type_declaration{
            .name = ast::identifier("Real"),
            .type = ast::type(ast::type_builtin::real),
        }
    );
    add_type( // NOLINT
        ast::type_declaration{
            .name = ast::identifier("Char"),
            .type = ast::type(ast::type_builtin::character),
        }
    );
    auto bool_constants = ast::group<ast::handle<ast::identifier>>();
    bool_constants.push_back(ast::handle(ast::identifier("False")));
    bool_constants.push_back(ast::handle(ast::identifier("True")));
    add_type( // NOLINT
        ast::type_declaration{
            .name = ast::identifier("Boolean"),
            .type = ast::type(ast::enumerated_type{
                .enum_members = std::move(bool_constants)
            })
        }
    );

}

auto semantic_context::try_get_type_id(std::string_view name) const
    -> std::optional<ast::type_id> {
    return names().lookup(name)
        .and_then([&](ast::tagged_id sym) 
            -> std::optional<ast::type_id> {
            if (sym.kind != ast::symbol_kind::type) return std::nullopt;
            return static_cast<ast::type_id>(sym.id);
        });
}
auto semantic_context::get_integer_id() const
    -> ast::type_id { return ast::type_id{1}; }
auto semantic_context::get_real_id() const
    -> ast::type_id { return ast::type_id{2}; }
auto semantic_context::get_bool_id() const
    -> ast::type_id { return ast::type_id{4}; }
auto semantic_context::get_char_id() const
    -> ast::type_id { return ast::type_id{3}; }
auto semantic_context::maybe_sploink_enum(type_observer type)
    -> semantic_result<void> {
    auto maybe_sploinker = [&](ast::type const& type_)
        -> type_observer {
        // TODO: find a way to handle errors
        visit(overloaded{
            [](auto const&) {},
            [&](ast::enumerated_type const& e) {
                for (int i = 0; i < e.enum_members.size(); ++i) {
                    auto _ = add_constant(ast::constant_declaration{
                        .name = e.enum_members[i].value_or("##poisoned enum value"),
                        .constant = ast::enum_constant{
                            .type = type,
                            .ord_value = i
                        }
                    });
                }
            }
        }, type_);
        return poison_pill;
    };

    type.and_then(maybe_sploinker);

    return {};
}
auto semantic_context::add_type(ast::type_declaration type_decl) 
    -> semantic_result<void> {
    auto const [is_success, it] = names().insert(
        type_decl.name, 
        {
            .id = 0,
            .kind = ast::symbol_kind::type, 
        }
    );
    auto _ = maybe_sploink_enum(type_decl.type);
    if (is_success) {
        auto const id = table().add(std::move(type_decl));
        it->second.id = std::to_underlying(id);
    } else {
        it->second.id = 0;
    }
    return {};
}

auto semantic_context::add_constant(ast::constant_declaration const_decl)
    -> semantic_result<void> { 
    auto const [is_success, it] = names().insert(
        const_decl.name, 
        {
            .id = 0,
            .kind = ast::symbol_kind::constant,
        }
    );
    if (is_success) {
        auto const id = table().add(std::move(const_decl));
        it->second.id = std::to_underlying(id);
    }

    return {};
}
auto semantic_context::add_variable(ast::variable_declaration var_decl)
    -> semantic_result<ast::variable_id> {
    auto res = semantic_result<ast::variable_id>();
    res.set_result(ast::variable_id::poison);
    auto const [is_success, it] = names().insert(
        var_decl.name, 
        {
            .id = 0,
            .kind = ast::symbol_kind::variable, 
        }
    );
    if (is_success) {
        auto const id = table().add(std::move(var_decl));
        it->second.id = std::to_underlying(id);
        res.set_result(id);
    }
    
    return res;   
}

auto semantic_context::synthesize_dummy_expression() const
    -> ast::expression {
    return ast::constant_name{
        .type = type_from_id(ast::type_id::poison),
        .id   = ast::constant_id::poison,
    };
}

template<typename R, typename T>
concept view_of = 
    std::ranges::view<R> && 
    std::same_as<
        std::ranges::range_value_t<R>, 
        T
    >;
auto check_overlap() {}
auto semantic_context::expose_records_fields(
    std::span<ast::variable_id> v_ids
)   -> semantic_result<void> {
    for (auto v_id : v_ids) {
        auto type = type_from_id(v_id);
        auto record_type = ast::get_if<ast::record_type>(type);
        auto const alt = ast::handle_group<ast::fixed_field>();
        auto const& fields = record_type
            .transform(member(&ast::record_type::fixed_fields))
            .ref_or(alt);
        for (auto&& field_h : fields) {
            if (field_h.is_poisoned()) continue;
            auto& field = field_h.unsafe_value();
            auto _  = add_variable(ast::variable_declaration{
                .name = field.name,
                .type = ast::clone(field.type),
            });
        }
    }
    return {};
}

namespace {
template<typename T>
inline constexpr auto extract_type_from_handle = 
    [](ast::observer_handle<T> arg) 
        -> ast::observer_handle<ast::type> {
        return arg.and_then([](T const& arg_) 
            -> ast::observer_handle<ast::type> {
            return arg_.type;
        });
    };
struct type_matcher {
    auto match(
        ast::observer_handle<ast::type> type1,
        ast::observer_handle<ast::type> type2
    ) const -> semantic_result<bool> {
        // if (type1.is_poisoned() or type2.is_poisoned()) return {};
        return then_all(cure(*this), type1, type2);
        // return (*this)(type1.unsafe_value(), type2.unsafe_value());
    }

    auto operator()(ast::type const& type1, ast::type const& type2) const
        -> semantic_result<bool> { return std::visit(*this, type1, type2); }
    auto operator()(auto const& type1, ast::type const& type2) const 
        -> semantic_result<bool> { return (*this)(type2, type1); }
    auto operator()(ast::type const& type1, auto const& type2) const
        -> semantic_result<bool> { 
        return std::visit(
            [&type2, this](auto const& type1_) { return (*this)(type1_, type2); },
            type1
        );
    }

    auto operator()(ast::type_identifier const& type1, ast::type_identifier const& type2) const 
        -> semantic_result<bool> {
        auto res = semantic_result<bool>();
        if (type1.id == type2.id) return {};
        else res.add_error(contextual_error());
        return res;
    }
    auto operator()(ast::type_identifier const& type1, auto const& type2) const 
        -> semantic_result<bool> {
        auto underlying_type_obs = ctx.type_from_id(type1.id);
        if (underlying_type_obs.is_poisoned()) return {};
        else return (*this)(underlying_type_obs.unsafe_value(), type2);
    }
    auto operator()(auto const& type1, ast::type_identifier const& type2) const
        -> semantic_result<bool> {
        return (*this)(type2, type1);
    }
    auto operator()(ast::array_type const& type1, ast::array_type const& type2) const
        -> semantic_result<bool> {
        auto res = semantic_result<bool>();
        res.set_result(true);
        if (not res.sieve(match(type1.component_type, type2.component_type))) {
            res.set_result(false);
        }
        if (type1.index_types.size() != type2.index_types.size()) {
            res.add_error(dummy_error(
                {}, "argument count missmatch"
            ));
            res.set_result(false);
            return res;
        }
        for (std::size_t i = 0; i < type1.index_types.size(); ++i) {
            if (not res.sieve(match(type1.index_types[i], type2.index_types[i]))) {
                res.set_result(false);
            }
        }
        return res;
    }
    auto operator()(ast::set_type const& type1, ast::set_type const& type2) const
        -> semantic_result<bool> {
        auto underlying_type_obs1 = ast::observer_handle(type1.base);
        auto underlying_type_obs2 = ast::observer_handle(type2.base);
        return match(underlying_type_obs1, underlying_type_obs2);
    }
    auto operator()(ast::function_type const& type1, ast::function_type const& type2) const
        -> semantic_result<bool> {
        auto res = semantic_result<bool>();
        res.set_result(true);
        if (not res.sieve(match(type1.return_type, type2.return_type))) {
            res.set_result(false);
        }
        if (type1.arguments.size() != type2.arguments.size()) {
            res.set_result(false);
            res.add_error(dummy_error(
                {}, "not enough arguments"
            ));
            return res;
        }
        auto arg_types1 = type1.arguments 
            | std::views::transform(extract_type_from_handle<ast::argument>);
        auto arg_types2 = type2.arguments
            | std::views::transform(extract_type_from_handle<ast::argument>);
        for (std::size_t i = 0; i < type1.arguments.size(); ++i) {
            if (not res.sieve(match(arg_types1[i], arg_types2[i]))) {
                res.set_result(false);
            }
        }
        return res;
    }
    auto operator()(ast::procedure_type const& type1, ast::procedure_type const& type2) const
        -> semantic_result<bool> {
        auto res = semantic_result<bool>();
        res.set_result(true);
        if (type1.argument_types.size() != type2.argument_types.size()) {
            res.set_result(false);
            res.add_error(dummy_error(
                {}, "argument count missmatch"
            ));
            return res;
        }
        for (std::size_t i = 0; i < type1.argument_types.size(); ++i) {
            if (not res.sieve(match(type1.argument_types[i], type2.argument_types[i]))) {
                res.set_result(false);
            }
        }
        return res;
    }
    auto operator()(ast::subrange_type const& type1, ast::subrange_type const& type2) const
        -> semantic_result<bool> {
        
    }
    auto operator()(ast::type_builtin const& type1, ast::type_builtin const& type2) const
        -> semantic_result<bool> {
        auto res = semantic_result<bool>();
        res.set_result(true);
        if (type1 != type2) {
            res.set_result(false);
            res.add_error(dummy_error(
                {}, "type missmatch"
            ));
        }
        return res;
    }

    //catch all
    auto operator()(auto const& type1, auto const& type2) const
        -> semantic_result<bool> {
        auto res = semantic_result<bool>();
        res.set_result(false);
        res.add_error(dummy_error(
            {}, "type missmatch"
        ));
        return res;
    }


    semantic_context const& ctx;
};
}


auto semantic_context::match_types(type_observer type1,  type_observer type2) const 
    -> semantic_result<bool> {
    return type_matcher{*this}.match(type1, type2);
}


//
/// Named lookup
//

auto semantic_context::try_get_variable_id(ast::identifier_view name) const
    -> ast::maybe<ast::variable_id> {
    return names().lookup(name).and_then([](ast::tagged_id sym) 
        -> ast::maybe<ast::variable_id> {
        if (sym.kind != ast::symbol_kind::variable) return std::nullopt;
        return static_cast<ast::variable_id>(sym.id);
    });
}
auto semantic_context::try_get_function_id(ast::identifier_view name) const
    -> ast::maybe<ast::function_id> {
    return names().lookup(name).and_then([](ast::tagged_id sym) 
        -> ast::maybe<ast::function_id> {
        if (sym.kind != ast::symbol_kind::function) return std::nullopt;
        return static_cast<ast::function_id>(sym.id);
    });
}
auto semantic_context::try_get_constant_id(ast::identifier_view name) const
    -> ast::maybe<ast::constant_id> {
    return names().lookup(name).and_then([](ast::tagged_id sym) 
        -> ast::maybe<ast::constant_id> {
        if (sym.kind != ast::symbol_kind::constant) return std::nullopt;
        return static_cast<ast::constant_id>(sym.id);
    });
}

auto semantic_context::get_ast_node(ast::variable_id id) const
    -> ast::observer_handle<ast::variable_declaration> {
    return table().get(id);
}
auto semantic_context::get_ast_node(ast::function_id id) const 
    -> ast::observer_handle<ast::function_declaration> {
    return table().get(id);
}
auto semantic_context::get_ast_node(ast::constant_id id) const 
    -> ast::observer_handle<ast::constant_declaration> {
    return table().get(id);
}

auto semantic_context::get_ast_node(ast::type_id id) const
    -> ast::observer_handle<ast::type_declaration> {
    return table().get(id);
}

auto semantic_context::type_from_id(ast::variable_id id) const
    -> type_observer {
    return get_ast_node(id).and_then(
        [](ast::variable_declaration const& decl) -> type_observer { return decl.type; }
    );
}
auto semantic_context::type_from_id(ast::constant_id id) const
    -> type_observer {
    fmt::println("bad id:{}", std::to_underlying(id));
    return get_ast_node(id).and_then(
        [&](ast::constant_declaration const& decl) -> type_observer { 
            return get_constant_type(decl.constant);
        }
    );
}
auto semantic_context::type_from_id(ast::function_id id) const
    -> type_observer {
    return get_ast_node(id).and_then(
        [](ast::function_declaration const& decl) -> type_observer { return decl.type; }
    );
}

auto semantic_context::type_from_id(ast::type_id id) const
    -> type_observer {
    return get_ast_node(id).and_then(
        [](ast::type_declaration const& decl) -> type_observer { return decl.type; }
    );
}


auto semantic_context::get_expression_type(ast::observer_handle<ast::expression> exp) const
    -> ast::observer_handle<ast::type> {
    return exp.and_then(
        [this](ast::expression const& e) { return this->get_expression_type(e); }
    );
}
auto semantic_context::get_expression_type(ast::observer_handle<ast::expression_leaf> exp) const
    -> type_observer { return exp.and_then(LIFT_MEMBER(get_expression_type)); }
auto semantic_context::get_expression_type(ast::expression_leaf const& exp) const 
    -> type_observer {
    return std::visit(overloaded{
            [&](ast::expression_atom const& name) { 
                return std::visit(overloaded{
                    [](auto const& name_) { return name_.type; },
                    [&](ast::constant const& name_) { return get_constant_type(name_); }
                }, name);
            },
            [](auto const& expr) { return expr.type; }
        }, exp
    );
}
auto semantic_context::get_expression_type(ast::expression const& exp) const
    -> ast::observer_handle<ast::type> {
    return std::visit(overloaded{
        [this](ast::expression_leaf const& e) { return this->get_expression_type(e); },
        [](auto const& e) -> ast::observer_handle<ast::type> { return e.type; }
    }, exp);
}
auto semantic_context::get_constant_type(ast::constant const& cnst) const
    -> type_observer {
    // TODO: proper types 
    // can't do now since havent introduced Int and Real in the symbol-table
    return poison_pill;
}
auto semantic_context::get_constant_type(ast::observer_handle<ast::constant> cnst) const
    -> type_observer {
    return cnst.and_then(LIFT_MEMBER(get_constant_type));
}
auto semantic_context::get_function_from_type(type_observer function) const
    -> semantic_result<ast::observer_handle<ast::function_type>> {
    auto res = semantic_result<ast::observer_handle<ast::function_type>>();
    res.set_result(ast::get_if<ast::function_type>(function));
    if (not ast::is<ast::function_type>(function)) {
        res.add_error(dummy_error({}, "expected a function type"));
    }
    return res;
}

auto semantic_context::call_type(
    type_observer type, 
    std::span<type_observer> caller_args
) const -> semantic_result<type_observer> {
    using namespace ast;
    namespace stdv = std::views;
    auto res = semantic_result<type_observer>();
    auto func_handle = res.sieve(get_function_from_type(type));
    res.set_result(try_member<&function_type::return_type>(func_handle));
    auto argument_handles = try_member<&function_type::arguments>(func_handle);
    argument_handles.and_then(
        [&](handle_group<argument> const& callee_args) 
            -> drainage {
            auto callee_args_ = callee_args
                |stdv::transform(try_member<&argument::type>);
            if (callee_args.size() != caller_args.size()) {
                res.add_error(dummy_error({}, "hewwo uwu 2"));
                res.set_result(poison_pill);
                return {};
            }
            for (std::size_t i = 0; i < callee_args.size(); ++i) {
                if (res.sieve(match_types(callee_args_[i], caller_args[i]))) {
                    res.set_result(poison_pill);
                }
            }
            return {};
    });
    return res;
}

auto semantic_context::get_array_from_type(type_observer array) const
    -> semantic_result<ast::observer_handle<ast::array_type>> {
    auto res = semantic_result<ast::observer_handle<ast::array_type>>();
    res.set_result(ast::get_if<ast::array_type>(array));
    if (not ast::is<ast::array_type>(array)) {
        res.add_error(dummy_error({}, "expected an array"));
    }
    return res;
}

auto semantic_context::index_type(
    type_observer type, 
    std::span<type_observer> indexer_args
) const -> semantic_result<type_observer> {
    using namespace ast;
    auto res = semantic_result<type_observer>();
    auto array_handle = res.sieve(get_array_from_type(type));
    res.set_result(try_member<&array_type::component_type>(array_handle));
    auto indexee_args = try_member<&array_type::index_types>(array_handle);
    indexee_args.and_then(
        [&](ast::handle_group<ast::type> const& indexees)
            -> drainage {
            if (indexees.size() != indexer_args.size()) {
                res.add_error(dummy_error({}, "hewwo uwu"));
                res.set_result(poison_pill);
                return {};
            }
            for (std::size_t i = 0; i < indexees.size(); ++i) {
                if (res.sieve(match_types(indexees[i], indexer_args[i]))) {
                    res.set_result(poison_pill);
                }
            }
            return {};
    });
    
    return res;
}

auto semantic_context::get_record_from_type(type_observer type) const
    -> semantic_result<ast::observer_handle<ast::record_type>> {
    auto res = semantic_result<ast::observer_handle<ast::record_type>>();
    res.set_result(ast::get_if<ast::record_type>(type));
    if (not ast::is<ast::record_type>(type)) {
        res.add_error(dummy_error({}, "expected a record type"));
    }
    return res;
}

auto semantic_context::member_type(
    type_observer type,
    ast::identifier_view name
) const -> semantic_result<type_observer> {
    using namespace ast;
    auto res = semantic_result<type_observer>();
    res.set_result(poison_pill);
    auto fields = res.sieve(get_record_from_type(type))
        .transform(member(&record_type::fixed_fields))
        .and_then([&](handle_group<fixed_field> const& fields)->drainage{
            for (auto&& field : fields) {
                bool is_the_one = field
                    .transform(member(&fixed_field::name))
                    .and_then(cure(equal_to(name)));
                if (is_the_one) {
                    res.set_result(try_member<&fixed_field::type>(field));
                    return {};
                }
            }
            res.add_error(dummy_error({}, "no such member"));
            return {};
        });
    return res;
}

auto semantic_context::get_pointer_from_type(type_observer type) const
    -> semantic_result<ast::observer_handle<ast::pointer_type>> {
    auto res = semantic_result<ast::observer_handle<ast::pointer_type>>();
    res.set_result(ast::get_if<ast::pointer_type>(type));
    if (not ast::is<ast::pointer_type>(type)) {
        res.add_error(dummy_error({}, "expected a pointer type"));
    }
    return res;
}

auto semantic_context::dereference_type(type_observer type) const 
    -> semantic_result<type_observer> {
    auto res = semantic_result<type_observer>();
    auto ptr_handle = res.sieve(get_pointer_from_type(type));
    res.set_result(ast::try_member<&ast::pointer_type::base>(ptr_handle));
    return res;
}

auto semantic_context::add_types(type_observer lhs, type_observer rhs) const
    -> semantic_result<type_observer> {
    return {};
}

struct type_adder {
    // auto operator()(auto const&, auto const&)
    //     -> semantic_result<ast::observer_handle<ast::type>> {
    //     return std::unexpected(contextual_error());
    // }
    semantic_context const& ctx;
};

auto semantic_context::add_types(ast::type const& lhs, ast::type const& rhs) const
    -> semantic_result<type_observer> {
    // return std::visit(type_adder{*this}, lhs, rhs);
}

} // namespace fed
