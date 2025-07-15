#include "fed/parser/context.hpp"
#include "fed/parser/parser.hpp"
#include "fed/diagnostics/internal-error.hpp"
#include "fed/parser/parse_error.hpp"
#include "fed/parser/parser-helpers.hpp"
#include "fed/representations/ast.hpp"
#include "fed/representations/ast/forward.hpp"
#include "fed/representations/ast/nodes.hpp"
#include "fed/scanner/token.hpp"
#include "fed/utils/macros.hpp"
#include "fed/utils/predicates.hpp"
#include "fed/utils/superutil.hpp"
#include "fed/diagnostics/buffer.hpp"

#include <boost/charconv/chars_format.hpp>
#include <boost/charconv/from_chars.hpp>

#include <algorithm>
#include <fmt/base.h>
#include <iterator>
#include <optional>
#include <utility>

namespace fed {
// context is created outside the block and then moved in
auto parser::parse_block(semantic_context ctx)
    -> parse_result<ast::block> {
    if (current_token_is(equal_to(token_type::keyword_const))) {

    }
    if (current_token_is(equal_to(token_type::keyword_type))) {
        // parse_type_definitions(ctx);
    }
    
    
}
//
/// TYPE DECLARATION PARSING

auto parser::parse_type_definitions(semantic_context& ctx)
    -> parse_result<void> {
    while (true) {
        using enum token_type;
        auto termination_tokens = std::array{
            keyword_var, keyword_begin,
            keyword_function, keyword_procedure
        };
        if (current_token_is(any_of(termination_tokens))) {
            break;
        }
        auto is_succ = parse_type_definition(ctx);
        if (is_succ) {
            auto _ = ctx.add_type(std::move(*is_succ))
                .transform_error(LIFT_MEMBER(push_error));
        } else {
            return {};
        }
        TRY_PARSE_TOKEN(token_type::semicolon);
    }
}
auto parser::parse_type_definition(semantic_context& ctx)
    -> parse_result<ast::type_declaration> {
    auto identifier_exp = parse_identifier(ctx);
    if (not identifier_exp.has_value()) {
        return std::unexpected(std::move(identifier_exp.error()));
    }
    auto err = consume_and_advance_expecting(token_type::equal);
    if (not err.has_value()) {
        return std::unexpected(std::move(err).error());
    }
    auto type_exp = parse_type(ctx);
    if (not type_exp.has_value()) {
        return std::unexpected(std::move(type_exp).error());
    }
    
    return ast::type_declaration{
        .name = *std::move(identifier_exp),
        .type = *std::move(type_exp),
    };
}

auto parser::parse_type(semantic_context const& ctx)
    -> parse_result<ast::type> {

    switch (current_token().type()) {
        case token_type::keyword_array: {
            return parse_array_type(ctx)
                .transform(construct<ast::type>);
        }
        case token_type::keyword_set: {
            return parse_set_type(ctx)
                .transform(construct<ast::type>);
        }
        case token_type::keyword_file: {
            return parse_file_type(ctx)
                .transform(construct<ast::type>);
        }
        case token_type::keyword_record: {
            return parse_record_type(ctx)
                .transform(construct<ast::type>);
        }
        case token_type::keyword_function: {
            return parse_function_type(ctx)
                .transform(construct<ast::type>);
        }
        case token_type::caret: {
            return parse_pointer_type(ctx)
                .transform(construct<ast::type>);
        }
        case token_type::l_paren: {
            return parse_enumerated_type(ctx)
                .transform(construct<ast::type>);
        }
        case token_type::identifier: {
            if (ctx.try_get_type_id(current_token().view().base()).has_value()) {
                return parse_type_identifier(ctx)
                    .transform(construct<ast::type>);
            } else if (ctx.try_get_constant_id(current_token().view().base()).has_value()) {
                [[fallthrough]];
            } else {
                return std::unexpected(parse_error());
            }
        }
        case token_type::number_real:
        case token_type::number_integer:
        case token_type::literal: {
            return parse_subrange_type(ctx)
                .transform(construct<ast::type>);
        }
        default: return std::unexpected(parse_error());
    }
}

auto parser::parse_enumerated_type(semantic_context const& ctx) 
    -> parse_result<ast::enumerated_type> {
    if (auto succ = consume_and_advance_expecting(token_type::l_paren);
        not succ.has_value()) {
        return std::unexpected(std::move(succ).error());
    }
    auto idents = parse_many(&parser::parse_identifier, ctx, parse_parameters{
        .separator = token_type::comma,
        .success_terminators = {token_type::r_paren},
        .hazard_terminators = {}
    });
    return ast::enumerated_type{
        .enum_members = std::move(idents),
    };
}
auto parser::parse_subrange_type(semantic_context const& ctx)
    -> parse_result<ast::subrange_type> {
    using enum token_type;
    return chain_parse(
        ctx, default_action<ast::subrange_type>(*this),
        &parser::parse_constant, dotdot, &parser::parse_constant
    );
}
auto parser::parse_type_identifier(semantic_context const& ctx)
    -> parse_result<ast::type_identifier> {
    if (current_token().type() != token_type::identifier) {
        return std::unexpected(parse_error());
    }
    auto token = consume_and_advance().view();
    auto id_opt = ctx.try_get_type_id(token.base());

    if (id_opt.has_value()) {
        return ast::type_identifier{.id = id_opt.value()};
    }

    return std::unexpected(parse_error());
}

auto parser::parse_pointer_type(semantic_context const& ctx)
    -> parse_result<ast::pointer_type> {
    return consume_and_advance_expecting(token_type::caret)
        .and_then([&](){ return parse_type(ctx); })
        .transform(construct<ast::pointer_type>);
}

auto parser::parse_array_type(semantic_context const& ctx) 
    -> parse_result<ast::array_type> {
    auto action = [this](
        ast::group<ast::handle<ast::type>> indices,
        parse_result<ast::type> component_type
    ) {
        return ast::array_type{
            .index_types = std::move(indices),
            .component_type = std::move(component_type)
                .transform_error(LIFT_MEMBER(push_error))
                .transform(construct<ast::handle<ast::type>>)
                .value_or(poison_pill)
        };
    };
    using enum token_type;
    auto parse_type_list = [](auto&& parser, semantic_context const& ctx) {
        return parser.some_parse(ctx, &parser::parse_type, comma);
    };

    return chain_parse(
        ctx, action,
        keyword_array, l_square, parse_type_list, r_square,
        keyword_of, &parser::parse_type
    );
 }

auto parser::parse_argument(semantic_context const& ctx)
    -> parse_result<ast::argument> {
    auto kind = maybe_consume_and_advance_expecting(token_type::keyword_var)?
        ast::argument_kind::ref :
        ast::argument_kind::copy;
    auto identifier = current_token_is(equal_to(token_type::identifier))? 
        ast::maybe<ast::identifier>(consume_and_advance().view().base()) :
        std::nullopt;
    if (auto success = consume_and_advance_expecting(token_type::colon);
        not success.has_value()) {
        return std::unexpected(std::move(success).error());
    }
    auto type = parse_type(ctx)
        .transform_error(LIFT_MEMBER(push_error))
        .transform(construct<ast::handle<ast::type>>)
        .value_or(poison_pill);

    return ast::argument{
        .name = std::move(identifier),
        .type = std::move(type),
        .kind = kind,
    };
}

auto parser::parse_function_type(semantic_context const& ctx)
    -> parse_result<ast::function_type> {
    auto action = [this](
        ast::group<ast::handle<ast::argument>> arguments,
        parse_result<ast::type> return_type
    ) {
        return ast::function_type{
            .return_type = contaminate(*this, std::move(return_type)),
            .arguments = std::move(arguments),
        };
    };
    using enum token_type;
    auto argument_list_parse = [](auto&& parser, semantic_context const& ctx) {
        return parser.many_parse(ctx, &parser::parse_argument, comma, r_paren);
    };
    return chain_parse(
        ctx, action,
        keyword_function, l_paren, argument_list_parse, r_paren,
        colon, &parser::parse_type
    );
}

auto parser::parse_fixed_part(semantic_context const& ctx)
    -> parse_result<ast::fixed_part> {
    auto fixed_field_parse = [](
        parser& parser, 
        semantic_context const& ctx
    ) -> ast::group<ast::handle<ast::fixed_field>> {
        auto result = ast::group<ast::handle<ast::fixed_field>>();
        auto ids = parser.parse_many(&parser::parse_identifier, ctx, {
            .separator = token_type::comma,
            .success_terminators = {token_type::colon},
            .hazard_terminators  = {},
        });
        parser.consume_and_advance_expecting(token_type::colon);
        auto type = parser.parse_type(ctx)
            .transform(construct<ast::handle<ast::type>>)
            .value_or(poison_pill);
        auto clone_type_for_field = [&type](ast::identifier&& idnt) 
            -> ast::fixed_field { 
            return {
                .name = std::move(idnt), 
                .type = ast::clone(type),
            };
        };
        auto move_type_for_field = [&type](ast::identifier&& idnt) 
            -> ast::fixed_field { 
            return {
                .name = std::move(idnt), 
                .type = std::move(type),
            };
        };
        for (auto i = std::size_t(0); i < ids.size(); ++i) {
            auto const is_last_index = i == ids.size() - 1;
            // why did i do that?
            // because i didn't want an extra copy
            if (is_last_index) {
                result.push_back(
                    std::move(ids[i]).transform(move_type_for_field)
                );
            } else {
                result.push_back(
                    std::move(ids[i]).transform(clone_type_for_field)
                );
            }
        }
        return result;
    }; 
    auto result = ast::fixed_part();
    while (true) {
        if (current_token_is(equal_to(token_type::keyword_end))) {
            break;
        }
        std::ranges::move(
            fixed_field_parse(*this, ctx),
            std::back_inserter(result)
        );
        if (current_token_is(equal_to(token_type::keyword_end))) {
            break;
        }
        consume_and_advance_expecting(token_type::semicolon);
    }

    return result;
}
auto parser::parse_record_type(semantic_context const& ctx)
    -> parse_result<ast::record_type> {
    if (current_token_is(not equal_to(token_type::keyword_record))) {
        return std::unexpected(parse_error());
    }
    consume_and_advance();
    auto fixed_fields = *parse_fixed_part(ctx);
    if (auto success = consume_and_advance_expecting(token_type::keyword_end);
        not success.has_value()) {
        return std::unexpected(std::move(success).error());
    }

    return ast::record_type{
        .fixed_fields = std::move(fixed_fields),
    };
}
//
// auto parser::parse_field_list()
//     -> parse_result<handle<ast::record_type>> {
//     // auto result = ast::record_type();
//     //
//     // TRY(result.fixed_fields, parse_group_of_symbols(
//     //     *this, &parser::parse_fixed_field, 
//     //     {token_type::keyword_end, token_type::keyword_case}, 
//     //     token_type::empty)
//     // );
//     // TRY(result.variant_part, parse_maybe(*this, &parser::parse_variant_part, token_type::keyword_case));
//     //
//     // return result;
// }
//
// auto parser::parse_fixed_field()
//     -> parse_result<handle<ast::fixed_fields>> {
//     // auto result = ast::fixed_field();
//     //
//     // TRY(result.names, parse_group_of_symbols(
//     //     *this, &parser::parse_identifier, 
//     //     {token_type::colon}, token_type::comma)
//     // );
//     // TRY_OPT(consume_and_advance_expecting(token_type::colon));
//     // TRY(result.type, parse_type());
//     // TRY_OPT(consume_and_advance_expecting(token_type::semicolon));
//     //
//     // return result;
// }
//
// auto parser::parse_variant_part()
//     -> parse_result<handle<ast::variant_field>> {
//     auto result = ast::variant_field();
//
//     TRY_OPT(consume_and_advance_expecting(token_type::keyword_case));
//     // TODO: make this identifier optional
//     // btw grammar for this optional identifier sucks
//     TRY(result.name, parse_identifier());
//     TRY_OPT(consume_and_advance_expecting(token_type::colon));
//     TRY(result.tag, parse_type());
//     TRY(result.variants, parse_group_of_symbols(
//         *this, &parser::parse_variant, 
//         {token_type::keyword_end}, token_type::empty)
//     );
//     
//     return result;
// }
//
// auto parser::parse_variant()
//     -> parse_result<handle<ast::variant_part>> {
//     // auto result = ast::variant();
//     // auto result_fields = ast::record_type();
//     // 
//     // TRY(result.matches, parse_group_of_symbols(
//     //     *this, &parser::parse_constant, 
//     //     {token_type::colon}, token_type::comma)
//     // );
//     // TRY_OPT(consume_and_advance_expecting(token_type::colon));
//     // TRY_OPT(consume_and_advance_expecting(token_type::l_paren));
//
//     // TRY(result_fields.fixed_fields, parse_group_of_symbols(
//     //     *this, &parser::parse_fixed_field, 
//     //     {token_type::r_paren, token_type::keyword_case}, 
//     //     token_type::empty)
//     // );
//     // if (current_token().type() == token_type::keyword_case) {
//     //     TRY_OPT(consume_and_advance_expecting(token_type::keyword_case));
//     //     // TODO: make this identifier optional
//     //     auto variant_part_res = ast::variant_field();
//     //     TRY(variant_part_res.name, parse_identifier());
//     //     TRY_OPT(consume_and_advance_expecting(token_type::colon));
//     //     TRY(variant_part_res.tag, parse_type());
//     //     TRY(variant_part_res.variants, parse_group_of_symbols(
//     //         *this, &parser::parse_variant, 
//     //         {token_type::r_paren}, token_type::empty)
//     //     );
//     //
//     //     result_fields.variant_part = std::move(variant_part_res);
//     //
//     // } else {
//     //     result_fields.variant_part = std::nullopt;
//     // }
//     // TRY_OPT(consume_and_advance_expecting(token_type::r_paren));
//     // result.fields = std::move(result_fields);
//     // return result;
// }
//
// auto parser::parse_enumerated_type()
//     -> parse_result<handle<ast::enumerated_type>> {
//     auto result = ast::enumerated_type();
//     //
//     // TRY_OPT(consume_and_advance_expecting(token_type::l_paren));
//     // TRY(result.identifiers, parse_group_of_symbols(
//     //     *this, &parser::parse_identifier, 
//     //     {token_type::r_paren}, token_type::comma)
//     // );
//     // TRY_OPT(consume_and_advance_expecting(token_type::r_paren));
//     //
//     // return result;
// }

auto parser::parse_set_type(semantic_context const& ctx)
    -> parse_result<ast::set_type> {
    using enum token_type;
    return chain_parse(
        ctx,
        default_action<ast::set_type>(*this), 
        keyword_set, keyword_of, &parser::parse_type
    );
}

auto parser::parse_file_type(semantic_context const& ctx)
    -> parse_result<ast::file_type> {
    using enum token_type;
    return chain_parse(
        ctx,
        default_action<ast::file_type>(*this), 
        keyword_file, keyword_of, &parser::parse_type
    );
}
//
//
// auto parser::parse_subrange_type()
//     -> parse_result<handle<ast::subrange_type>> {
//     auto result = ast::subrange_type();
// }
//
// auto parser::parse_procedure_declaration()
//     -> parse_result<handle<ast::procedure_declaration>> {
//     using namespace std::literals;
//     auto result = ast::procedure_declaration();
//     auto body = std::optional<ast::block>();
//     TRY(result.head, parse_procedure_heading());
//     TRY_OPT(consume_and_advance_expecting(token_type::semicolon));
//
//     
//     if (current_token().view().base() != "forward"sv) {
//         TRY(result.body, parse_block());
//     } else {
//         consume_and_advance();
//     }
//     TRY_OPT(consume_and_advance_expecting(token_type::semicolon));
//
//     return result;
// }
//
//
// auto parser::parse_procedure_heading()
//     -> parse_result<handle<ast::procedure_heading>> {
//     auto result = ast::procedure_heading();
//     TRY_OPT(consume_and_advance_expecting(token_type::keyword_procedure));
//     TRY(result.name, parse_identifier());
//     if (current_token().type() == token_type::l_paren) {
//         TRY(result.formal_parametr_list, parse_formal_parameter_list());
//     }
//     TRY_OPT(consume_and_advance_expecting(token_type::r_paren));
//
//     return result;
// }
//
// auto parser::parse_function_declaration()
//     -> parse_result<handle<ast::function_declaration>> {
//     using namespace std::literals;
//     auto result = ast::function_declaration();
//     auto body = std::optional<ast::block>();
//     TRY(result.head, parse_function_heading());
//     TRY_OPT(consume_and_advance_expecting(token_type::semicolon));
//
//     // TODO: forward declarations
//     
//     if (current_token().view().base() != "forward"sv) {
//         TRY(result.body, parse_block());
//     } else {
//         consume_and_advance();
//     }
//     TRY_OPT(consume_and_advance_expecting(token_type::semicolon));
//
//     return result;
// }
//
// auto parser::parse_function_heading()
//     -> parse_result<handle<ast::function_heading>> {
//     auto result = ast::function_heading();
//     TRY_OPT(consume_and_advance_expecting(token_type::keyword_function));
//     TRY(result.name, parse_identifier());
//     if (current_token().type() == token_type::l_paren) {
//         TRY(result.formal_parametr_list, parse_formal_parameter_list());
//     }
//     TRY_OPT(consume_and_advance_expecting(token_type::r_paren));
//     TRY_OPT(consume_and_advance_expecting(token_type::colon));
//     TRY(result.return_type, parse_type());
//
//     return result;
// }
//
// auto parser::parse_formal_parameter_list()
//     -> parse_result<ast::group<handle<ast::formal_parameter>>> {
//     auto result = ast::group<handle<ast::formal_parameter>>();
//     TRY_OPT(consume_and_advance_expecting(token_type::l_paren));
//     TRY(result, parse_group_of_symbols(
//         *this, &parser::parse_formal_parameter, 
//         {token_type::r_paren}, token_type::semicolon));
//     TRY_OPT(consume_and_advance_expecting(token_type::r_paren));
//
//     return result;
// }
// auto parser::parse_formal_parameter()
//     -> parse_result<handle<ast::formal_parameter>> {
//     auto result = handle<ast::formal_parameter>();
//     switch(current_token().type()) {
//         using enum token_type;
//         case keyword_function:  TRY(result, parse_function_heading()); break;
//         case keyword_procedure: 
//         default: TRY(result, parse_formal_parameter_simple()); break;
//     }
//
//     return result;
// }
// auto parser::parse_formal_parameter_simple() 
//     -> parse_result<handle<ast::formal_parameter_simple>> {
//     auto result = ast::formal_parameter_simple();
//     result.is_variable = maybe_consume_and_advance_expecting(token_type::keyword_var);
//     // TRY(result.names, parse_group_of_symbols(
//     //     *this, &parser::parse_identifier, 
//     //     {token_type::colon}, token_type::comma) 
//     // );
//     TRY_OPT(consume_and_advance_expecting(token_type::colon));
//     TRY(result.type, parse_identifier());
//
//     return result;
// }
//
// auto parser::parse_variable_declaration()
//     -> parse_result<handle<ast::variable_declaration>> {
//     auto result = handle<ast::variable_declaration>();
//     // TRY(result->identifiers, parse_group_of_symbols(
//     //     *this, &parser::parse_identifier, {token_type::colon}, token_type::comma
//     // ));
//     TRY_OPT(consume_and_advance_expecting(token_type::colon));
//     TRY(result->type, parse_type());
//     
//     // for (auto const& identifier : result->identifiers) {
//     //     auto error_opt = m_context.insert(identifier->view.base(), result.get());
//     //     if (error_opt.has_value()) {
//     //         m_diagnostics.push_back(*error_opt);
//     //     }
//     // }
//
//     return result;
// }
//
// // seperated into a function because it gets repetetive
// // actual identifier "parsing" happens while lexing
// // identifier = [a-zA-z][a-zA-Z0-9]*
auto parser::parse_identifier(semantic_context const&)
    -> parse_result<ast::identifier> {
    auto const start = cursor();
    if (current_token().type() != token_type::identifier) {
        return std::unexpected(
            parse_error(
                start.where(), current_token().view(),
                parse_error::type::unexpected_token,
                token_type::identifier
            )
        );
    }

    return ast::identifier{consume_and_advance().view().base()};
}

} // namespace fed 
