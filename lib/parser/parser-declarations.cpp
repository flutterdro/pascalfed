#include "fed/diagnostics/compile-error.hpp"
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

#include <__algorithm/ranges_fold.h>
#include <boost/charconv/chars_format.hpp>
#include <boost/charconv/from_chars.hpp>

#include <algorithm>
#include <fmt/base.h>
#include <iterator>
#include <optional>
#include <utility>
#include <variant>

namespace fed {
// context is created outside the block and then moved in
auto parser::parse_block(semantic_context ctx)
    -> parse_result<ast::block> {
    anchors().push(std::array{
        token_type::keyword_var,
        token_type::keyword_type,
        token_type::keyword_const
    });
    if (current_token_is(equal_to(token_type::keyword_const))) {
        consume_and_advance();

    }
    anchors().pop(token_type::keyword_const);
    if (current_token_is(equal_to(token_type::keyword_type))) {
        auto _ = parse_type_definitions(ctx);
    }
    anchors().pop(token_type::keyword_type);
    if (current_token_is(equal_to(token_type::keyword_var))) {
        consume_and_advance();

    }
    anchors().pop(token_type::keyword_var);

    while (true) {
        if (current_token_is(equal_to(token_type::keyword_function))){

        } else if (
            current_token_is(equal_to(token_type::keyword_procedure))
        ) {

        } else {
            break;
        }
    }
    
    
}

auto parser::parse_function(semantic_context& ctx)
    -> parse_result<ast::function> {
    auto func_ctx = ctx.make_local();

    auto action = [](
        auto idnt,
        auto arguments,
        auto return_type
    ) {
        return ast::function_type{
            .return_type = monad_to_handle(std::move(return_type)),
            .arguments = std::move(*arguments),
        };
    };
    auto action2 = [](auto&& type) {
        return FWD(type);
    };
    using enum token_type;
    auto argument_list_parse = make_many_parse(&parser::parse_argument, semicolon);
    auto return_type_parse = [&](auto&& parser, auto&& ctx_) {
        return chain_parse(ctx_, action2, colon, parse_type_f);
    };
    chain_parse(
        ctx, action,
        keyword_function, parse_identifier_f,
        l_paren, argument_list_parse, r_paren,
        return_type_parse
    );

    parse_block(std::move(func_ctx));
}

//
/// TYPE DECLARATION PARSING
//

auto parser::parse_type_definitions(semantic_context& ctx)
    -> parse_result<void> {
    auto definition_action = [&](auto name, auto type) {
        if (not name.has_value()) 
            return identity_monad<std::monostate>();
        ctx.add_type(ast::type_declaration{
            .name = std::move(*name),
            .type = monad_to_handle(std::move(type)),
        }).sploink(diagnostics());
        return identity_monad<std::monostate>();
    };
    auto definition_parse = make_chain_parse(
        definition_action,
        &parser::parse_identifier,
        token_type::equal,
        &parser::parse_type,
        token_type::semicolon
    );
    // many_parse(ctx, definition_parse, token_type::empty);
    chain_parse(
        ctx,
        [](auto&&...) { return identity_monad<std::monostate>(); },
        token_type::keyword_type, 
        make_many_parse(definition_parse, token_type::empty)
    );
    return {};
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
        default: return std::unexpected(dummy_error(cursor().where(), "bad type"));
    }
}

auto parser::parse_enumerated_type(semantic_context const& ctx) 
    -> parse_result<ast::enumerated_type> {
    auto action = [](auto enums) {
        return ast::enumerated_type{.enum_members = std::move(*enums)};
    };
    
    return chain_parse(
        ctx,
        action,
        token_type::l_paren, 
        make_many_parse(&parser::parse_identifier, token_type::comma),
        token_type::r_paren
    );
}
auto parser::parse_subrange_type(semantic_context const& ctx)
    -> parse_result<ast::subrange_type> {
    using enum token_type;
    return chain_parse(
        ctx, default_action<ast::subrange_type>,
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
    auto action = [](
        auto indices,
        auto component_type
    ) {
        return ast::array_type{
            .index_types = std::move(*indices),
            .component_type = monad_to_handle(std::move(component_type))
        };
    };
    using enum token_type;
    auto parse_type_list = make_some_parse(&parser::parse_type, comma);
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
    auto action = [](
        auto arguments,
        auto return_type
    ) {
        return ast::function_type{
            .return_type = monad_to_handle(std::move(return_type)),
            .arguments = std::move(*arguments),
        };
    };
    auto action2 = [](auto&& type) {
        return FWD(type);
    };
    using enum token_type;
    auto argument_list_parse = make_many_parse(&parser::parse_argument, semicolon);
    auto return_type_parse = [&, this](auto&& parser, auto&& ctx_) {
        return chain_parse(ctx_, action2, colon, &parser::parse_type);
    };
    return chain_parse(
        ctx, action,
        keyword_function, l_paren, argument_list_parse, r_paren,
        return_type_parse
    );
}

auto parser::parse_fixed_part(semantic_context const& ctx)
    -> parse_result<ast::fixed_part> {
    auto fixed_field_action = [](auto idnt_m, auto type_m) {
        return identity_monad(repopulate_identifiers<ast::fixed_field>(
            std::move(*idnt_m), monad_to_handle(std::move(type_m))
        ));
    };
    auto fixed_field_parse = 
         make_chain_parse(
            fixed_field_action,
            make_some_parse(&parser::parse_identifier, token_type::comma),
            token_type::colon,
            &parser::parse_type,
            token_type::semicolon
        );
    
    auto fields = *many_parse(ctx, fixed_field_parse, token_type::empty);
    auto field_fold = [](auto&& acc, auto&& val) {
        if (val.is_poisoned()) return std::move(acc);
        for (auto&& field : *val) {
            acc.push_back(std::move(field));
        }
        return std::move(acc);
    };
    return std::ranges::fold_left(fields, ast::fixed_part(), field_fold);
}
auto parser::parse_record_type(semantic_context const& ctx)
    -> parse_result<ast::record_type> {
    auto action = [](auto&& fields) {
        return ast::record_type{.fixed_fields = std::move(*fields) };
    };

    return chain_parse(
        ctx, action,
        token_type::keyword_record,
        &parser::parse_fixed_part,
        token_type::keyword_end
    );
}

auto parser::parse_set_type(semantic_context const& ctx)
    -> parse_result<ast::set_type> {
    using enum token_type;
    return chain_parse(
        ctx,
        default_action<ast::set_type>, 
        keyword_set, keyword_of, &parser::parse_type
    );
}

auto parser::parse_file_type(semantic_context const& ctx)
    -> parse_result<ast::file_type> {
    using enum token_type;
    return chain_parse(
        ctx,
        default_action<ast::file_type>, 
        keyword_file, keyword_of, &parser::parse_type
    );
}
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
