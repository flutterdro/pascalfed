#include "fed/diagnostics/internal-error.hpp"
#include "fed/parser/context.hpp"
#include "fed/parser/parser.hpp"
#include "fed/representations/ast.hpp"
#include "fed/representations/ast/nodes.hpp"
#include "fed/scanner/token.hpp"
#include "fed/utils/superutil.hpp"
#include <charconv>
#include <fmt/std.h>
#include <__expected/unexpected.h>
#include <fmt/base.h>
#include <fmt/ostream.h>
#include <utility>
#include <variant>

#define TRY(dest, exp) \
do {auto exp2 = exp;\
if (not exp2.has_value()) return std::unexpected{std::move(exp2.error())}; \
dest = std::move(exp2.value());} while(false)
#define TRY_OPT(opt) \
if (opt.has_value()) return std::unexpected{std::move(*opt)};


namespace fed {
inline constexpr auto token_to_operation = [](token_type type) {
    switch (type) {
        using enum token_type;
        case plus:                  return ast::binary_operation::add;
        case minus:                 return ast::binary_operation::substract;
        case star:                  return ast::binary_operation::multiply;
        case slash:                 return ast::binary_operation::real_divide;
        case keyword_div:           return ast::binary_operation::integer_divide;
        case keyword_mod:           return ast::binary_operation::modulo;
        case keyword_or:            return ast::binary_operation::or_;
        case keyword_and:           return ast::binary_operation::and_;
        case keyword_in:            return ast::binary_operation::in;
        case less_than:             return ast::binary_operation::less;
        case greater_than:          return ast::binary_operation::greater;
        case not_equal:             return ast::binary_operation::not_equal;
        case equal:                 return ast::binary_operation::equal;
        case less_or_equal_than:    return ast::binary_operation::less_or_equal;
        case greater_or_equal_than: return ast::binary_operation::greater_or_equal;
        default: std::unreachable();
    }
};

using ast::handle;
auto parser::parse_expression(
    semantic_context const& ctx,
    precedence::level threshold
)   -> parse_result<ast::expression> {
    if (current_token().type() == token_type::l_paren) {
        consume_and_advance();
        auto lhs = parse_expression(ctx)
            .transform_error(LIFT_MEMBER(push_error))
            .value_or(ctx.synthesize_dummy_expression());
        consume_and_advance_expecting(token_type::r_paren);
        // return parse_expression(std::move(lhs), precedence::lowest);
        return lhs;
    }
    auto lhs = 
        (threshold < precedence::highest ? 
            parse_expression(ctx, up(threshold)) :
            parse_expression_leaf(ctx))
        .transform_error(LIFT_MEMBER(push_error))
        .value_or(ctx.synthesize_dummy_expression());
    auto is_unary_operator = any_of(std::array{
        token_type::plus, token_type::minus, token_type::keyword_not
    });
    return parse_rhs(ctx, std::move(lhs), threshold);
}

auto parser::parse_unary_expression(semantic_context const& ctx)
    -> parse_result<ast::unary_expression> {
    auto token = consume_and_advance().type();

    auto token_to_operation = [](token_type type) -> ast::unary_operation {
        switch (type) {
            case token_type::plus:        return ast::unary_operation::identity;  
            case token_type::minus:       return ast::unary_operation::negation;
            case token_type::keyword_not: return ast::unary_operation::logical_negation;
            default: std::unreachable();
        }
    };

    auto operand = (
        current_token().type() == token_type::l_paren ?
            parse_expression(ctx) :
            parse_expression_leaf(ctx)
        )
        .transform_error(LIFT_MEMBER(push_error))
        .value_or(ctx.synthesize_dummy_expression());
    auto type = ctx.get_expression_type(operand);


    return ast::unary_expression{
        .type = type,
        .operand = std::move(operand),
        .operation = token_to_operation(token),
    };
}
inline constexpr auto is_binary_operator = [](token_type type) {
    return any_of(std::array{
            token_type::plus,
            token_type::minus,
            token_type::star,
            token_type::slash,
            token_type::keyword_div,
            token_type::keyword_mod,
            token_type::keyword_or,
            token_type::keyword_and,
            token_type::keyword_in,
            token_type::less_than,
            token_type::greater_than,
            token_type::not_equal,
            token_type::equal,
            token_type::less_or_equal_than,
            token_type::greater_or_equal_than,
    })(type);
};

inline constexpr auto binary_operator_precedence = [](token_type type) {
        switch (type) {
            using enum token_type;
            case plus:                  return parser::precedence::adding;
            case minus:                 return parser::precedence::adding;
            case keyword_or:            return parser::precedence::adding;
            case star:                  return parser::precedence::multiplying;
            case slash:                 return parser::precedence::multiplying;
            case keyword_div:           return parser::precedence::multiplying;
            case keyword_mod:           return parser::precedence::multiplying;
            case keyword_and:           return parser::precedence::multiplying;
            case keyword_in:            return parser::precedence::relational;
            case less_than:             return parser::precedence::relational;
            case greater_than:          return parser::precedence::relational;
            case not_equal:             return parser::precedence::relational;
            case equal:                 return parser::precedence::relational;
            case less_or_equal_than:    return parser::precedence::relational;
            case greater_or_equal_than: return parser::precedence::relational;
            default: std::unreachable();
        }
    };

auto parser::parse_expression(
    semantic_context const& ctx,
    ast::expression lhs, 
    precedence::level threshold
)   -> parse_result<ast::expression> {
    
    auto token = current_token().type();

    // You fucking moron. It accidentally works because of this
    // When it encounters r_paren it goes up as it should
    // why did I do this anyway?
    if (not is_binary_operator(token)) return lhs;

    if (auto new_threshold = binary_operator_precedence(token);
        new_threshold > threshold) {
        lhs = parse_expression(ctx, std::move(lhs), new_threshold)
            .transform_error(LIFT_MEMBER(push_error))
            .value_or(ctx.synthesize_dummy_expression());
    }
    if (not is_binary_operator(current_token().type())) return lhs;
    
    
    return parse_rhs(ctx, std::move(lhs), threshold);
}


auto parser::parse_rhs(
    semantic_context const& ctx,
    ast::expression lhs,
    precedence::level threshold
)   -> parse_result<ast::expression> {
    
    if (current_token_is(not is_binary_operator)) { 
        return lhs;
    }
    auto op_precedence   = binary_operator_precedence(current_token().type());
    if (op_precedence < threshold) {
        return lhs;
    }
    auto operation_token = consume_and_advance().type();
    auto rhs = parse_expression(ctx, up(op_precedence))
            .transform_error(LIFT_MEMBER(push_error))
            .value_or(ctx.synthesize_dummy_expression());
   return parse_rhs(ctx, ast::binary_expression{
        .type = poison_pill,
        .lhs  = std::move(lhs),
        .rhs  = std::move(rhs),
        .operation = token_to_operation(operation_token),
    }, op_precedence);
}

auto parser::parse_expression_leaf(semantic_context const& ctx) 
    -> parse_result<ast::expression> {
    auto const lookahead = current_token();
    switch (lookahead.type()) {
        case token_type::identifier: {
            return parse_expression_leaf(ctx,
                determine_name_type(ctx, consume_and_advance().view().base())
            );
        }
        //TODO:
        //case number_real 
        //case number_integer
        //case string literal 
        //case character literal 
        default: {
            return std::unexpected(parse_error());
        }
    }
}

auto parser::parse_expression_leaf(
    semantic_context const& ctx,
    ast::expression base
)   -> parse_result<ast::expression> {
    auto const lookahead = current_token();
    switch (lookahead.type()) {
        case token_type::caret: {
            consume_and_advance();
            return parse_dereferencing(ctx, std::move(base));
        }
        case token_type::l_paren: {
            consume_and_advance();
            return parse_call(ctx, std::move(base));
        }
        case token_type::l_square: {
            consume_and_advance();
            return parse_indexing(ctx, std::move(base));
        }
        case token_type::dot: {
            consume_and_advance();
            return parse_member_access(ctx, std::move(base));
        }
        default: {
            return base;
        }
    }
}

auto parser::parse_dereferencing(
    semantic_context const& ctx,
    ast::expression base)
    -> parse_result<ast::expression> {
    auto base_handle     = handle<ast::expression>(std::move(base));
    auto expression_type = [&] () -> ast::observer_handle<ast::type> {
        auto pointer_type = ctx.get_expression_type(base_handle);
        if (auto type_exp = ctx.dereference_type(pointer_type)) {
            return *type_exp;
        } else {
            diagnostics().push_back(type_exp.error());
            return poison_pill;
        }
    }();
    return parser::parse_expression_leaf(
        ctx,
        ast::dereferenced_expression{
            .type = expression_type,
            .ptr  = std::move(base_handle),
        }
    );
}

auto parser::parse_call(semantic_context const& ctx, ast::expression base)
    -> parse_result<ast::expression> {
    using enum token_type;
    static constexpr auto soft_terminators = std::array{
        comma, r_paren, semicolon
    };
    static constexpr auto hard_terminators = std::array{
        comma, r_paren, semicolon
    };
    auto base_handle       = ast::handle<ast::expression>(std::move(base));
    auto caller_args_types = ast::group<ast::observer_handle<ast::type>>();
    auto caller_args       = ast::group<ast::handle<ast::expression>>();
    while(true) {
        if (auto arg_exp = parse_expression(ctx)) {
            caller_args_types.push_back(
                ctx.get_expression_type(*arg_exp)
            );
            caller_args.push_back(std::move(*arg_exp));
        } else {
            diagnostics().push_back(arg_exp.error());
            caller_args_types.push_back(poison_pill);
            caller_args.push_back(poison_pill);
        }
        if (current_token_is(not any_of(soft_terminators))) {
             advance_until(any_of(hard_terminators));
        }
        if (current_token().type() == token_type::comma) {
            consume_and_advance();
            continue;
        } else if (current_token().type() == token_type::r_paren) {
            consume_and_advance();
            break;
        } else if (current_token().type() == token_type::semicolon) {
            // TODO: error missing ')'
            return std::unexpected(parse_error());
        } else {
            
        }
    }

    auto return_type = ctx
        .call_type(
            ctx.get_expression_type(base_handle),
            caller_args_types
        )
        .transform_error([&](auto&& err) {
            this->diagnostics().push_back(err);
            return err;
        })
        .value_or(poison_pill);
    
    return ast::called_expression{
        .type      = return_type,
        .callable  = std::move(base_handle),
        .arguments = std::move(caller_args),
    };
}

auto parser::parse_indexing(semantic_context const& ctx, ast::expression base) 
    -> parse_result<ast::expression> {
    using enum token_type;
    auto base_handle      = ast::handle<ast::expression>(std::move(base));
    auto index_args_types = ast::group<ast::observer_handle<ast::type>>();
    auto index_args       = ast::group<ast::handle<ast::expression>>();
    while(true) {
        if (auto arg_exp = parse_expression(ctx)) {
            index_args_types.push_back(
                ctx.get_expression_type(*arg_exp)
            );
            index_args.push_back(std::move(*arg_exp));
        } else {
            diagnostics().push_back(arg_exp.error());
            index_args_types.push_back(poison_pill);
            index_args.push_back(poison_pill);
        }
        if (current_token_is(not any_of(std::array{comma, r_square, semicolon}))) {
            // TODO: error recovery
        }
        if (current_token().type() == token_type::comma) {
            consume_and_advance();
            continue;
        } else if (current_token().type() == token_type::r_square) {
            consume_and_advance();
            break;
        } else if (current_token().type() == token_type::semicolon) {
            // TODO: error missing ']'
            return std::unexpected(parse_error());
        } else {
            
        }
    }

    auto return_type = ctx 
        .index_type(
            ctx.get_expression_type(base_handle),
            index_args_types
        )
        .transform_error([&](auto&& err) {
            this->diagnostics().push_back(err);
            return err;
        })
        .value_or(poison_pill);
    
    return ast::indexed_expression{
        .type     = return_type,
        .array    = std::move(base_handle),
        .indecies = std::move(index_args),
    };
}

using namespace std::literals;

auto parser::parse_member_access(semantic_context const& ctx, ast::expression base)
    -> parse_result<ast::expression> {
    auto base_handle = ast::handle<ast::expression>(std::move(base));
    auto base_type = ctx.get_expression_type(base);
    auto [name, member_type] = [&] () 
        -> std::pair<ast::identifier, ast::observer_handle<ast::type>> {
        if (auto identifier_exp = parse_identifier(ctx)) {
            auto member_type = ctx
                .member_type(base_type, *identifier_exp)
                .transform_error([&](auto&& err) {
                    diagnostics().push_back(err);
                    return std::monostate();
                })
                .value_or(poison_pill);
            return {std::move(*identifier_exp), member_type};
        } else {
            return {"##invalid"s, poison_pill};
        }
    }();

    return ast::membered_expression{
        .type   = member_type,
        .object = std::move(base_handle),
        .member = name,
    };
}

auto parser::determine_name_type(semantic_context const& ctx, ast::identifier_view name)
    -> ast::expression_atom {
    auto bundle_up = [=, &ctx, this](auto id) {
        return ast::expression_atom(
            std::in_place_type<ast::name_from_id_t<decltype(id)>>,
            ctx.type_from_id(id), id
        );
    };
    auto try_func = [=, &ctx, this](auto member_func) {
        return [=, &ctx, this]() { return (ctx.*member_func)(name).transform(bundle_up);};
    };
    auto try_guess_function = try_func(&semantic_context::try_get_function_id);
    auto try_guess_variable = try_func(&semantic_context::try_get_variable_id);
    auto try_guess_constant = [&, this]() {
         return ctx
            .try_get_constant_id(name)
            .transform([&](auto const id) -> ast::expression_atom {
                return ast::constant(
                    std::in_place_type<ast::constant_name>,
                    ctx.type_from_id(id), id
                );
            });
    };


    auto maybe_bare_name = try_guess_variable()
        .or_else(try_guess_function)
        .or_else(try_guess_constant);
        // or_else procedure 
        // or_else error handle

    if (maybe_bare_name.has_value()) {
        return *maybe_bare_name;
    } else {
        return ast::constant_name{
            .type = poison_pill,
            .id = ast::constant_id::poison,
        };
    }
}

auto parser::parse_integer()
    -> parse_result<ast::integer_literal> {
    if (current_token_is(not equal_to(token_type::number_integer))) {
        return std::unexpected(parse_error());
    }
    auto tok = consume_and_advance().view();
    auto number = int(0);
    auto [ptr, ec] = std::from_chars(tok.data(), tok.data() + tok.size(), number);
    if (ec == std::errc::invalid_argument) {
        throw fed::internal_error("wiwiwi");
    } else if (ec == std::errc::result_out_of_range) {
        return std::unexpected(parse_error());
    }
    return ast::integer_literal{
        .value = number,
    };
}

auto parser::parse_constant(semantic_context const& ctx)
    -> parse_result<ast::constant> {
    switch (current_token().type()) {
        case token_type::number_integer: {
            return parse_integer()
                .transform(construct<ast::constant>);
        }
        case token_type::number_real: {
            throw fed::internal_error("Unimplemented");
        }
        case token_type::identifier: {
            auto view = consume_and_advance().view().base();
            if (auto id = ctx.try_get_constant_id(view);
                id.has_value()) {
                return ast::constant_name{
                    .type = ctx.type_from_id(*id),
                    .id   = *id,
                };
            } else {
                return std::unexpected(parse_error());
            }
        }
        default: return std::unexpected(parse_error());
    }
}


}
