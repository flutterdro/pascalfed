#include "fed/parser/context.hpp"
#include "fed/parser/parser.hpp"
#include "fed/diagnostics/internal-error.hpp"
#include "fed/parser/parse_error.hpp"
#include "fed/representations/ast.hpp"
#include "fed/representations/ast/forward.hpp"
#include "fed/representations/ast/nodes.hpp"
#include "fed/scanner/token.hpp"
#include "fed/utils/macros.hpp"
#include "fed/utils/predicates.hpp"
#include "fed/utils/superutil.hpp"
#include "fed/diagnostics/buffer.hpp"

#include <__expected/unexpected.h>
#include <boost/charconv/chars_format.hpp>
#include <boost/charconv/from_chars.hpp>

#include <algorithm>
#include <charconv>
#include <fmt/base.h>
#include <initializer_list>
#include <optional>
#include <thread>
#include <type_traits>
#include <utility>
#include <variant>

// using fed::ast::handle;
//
namespace fed {
//
//
// // I kind of miss rust's operator ?
// #define TRY(dest, exp) \
// do {auto exp2 = exp;\
// if (not exp2.has_value()) return std::unexpected{std::move(exp2.error())}; \
// dest = std::move(exp2.value());} while(false)
// #define TRY_OPT(opt) \
// if (opt.has_value()) return std::unexpected{std::move(*opt)};
//
// using list_of_terminals = std::initializer_list<token_type>;
// template<typename SymbolT>
// using parse_function_ptr = auto (parser::*)() -> parse_result<handle<SymbolT>>;
// template<typename SymbolT>
// constexpr auto parse_group_of_symbols(
//     parser& parser, 
//     parse_function_ptr<SymbolT> symbol_parse_function,
//     list_of_terminals terminals,
//     token_type separator
// ) -> parse_result<ast::group<handle<SymbolT>>> {
//     auto result = ast::group<handle<SymbolT>>{};
//
//     auto const start = parser.cursor();
//
//     {
//         auto element = handle<SymbolT>{};
//         TRY(element, (parser.*symbol_parse_function)());
//         result.push_back(std::move(element));
//     }
//
//     // while (stdr::none_of(terminals, func::equal_to(parser.current_token().type()))) {
//     //     auto const end = parser.cursor();
//     //     if (separator != token_type::empty and
//     //         parser.consume_and_advance().type() != separator) {
//     //         return std::unexpected(
//     //             parse_error(
//     //                 end.where(), source::view(start, end), 
//     //                 parse_error::type::missing_token,
//     //                 separator
//     //             )
//     //         );
//     //     }
//     //     
//     //     auto element = handle<SymbolT>{};
//     //     TRY(element, (parser.*symbol_parse_function)());
//     //     result.push_back(std::move(element));
//     // }
//
//     
//     return result;
// }
//
// template<typename SymbolT>
// auto parse_maybe(parser& parser, parse_function_ptr<SymbolT> func, token_type trigger)
//     -> parse_result<std::optional<handle<SymbolT>>> {
//     if (parser.current_token().type() != trigger) return std::nullopt;
//
//     return (parser.*func)();
// }
//
//
// // program = program-heading ';' block
// auto parser::parse_program()
//     -> parse_result<handle<ast::program>> {
//     // std::expected inhibits rvo
//     // for now I don't give a fuck about perfomance
//     // could be an issue later
//     auto result = ast::program{};
//     auto const program_view_start = cursor();
//
//     TRY(result.head, parse_program_heading());
//     
//     auto const program_head_end = cursor();
//     if (consume_and_advance().type() != token_type::semicolon) {
//         return std::unexpected(
//             parse_error(
//                 program_head_end.where(), 
//                 source::view(program_view_start, program_head_end),
//                 parse_error::type::missing_token,
//                 token_type::semicolon
//             )
//         );
//     }
//     
//     TRY(result.body, parse_block());
//     
//     result.region = source::view(program_view_start, cursor());
//
//     return result;
// }
//
// // program-heading = 'program' identifier ['(' identifier-list ')']
// auto parser::parse_program_heading()
//     -> parse_result<handle<ast::program_heading>> {
//     auto result = ast::program_heading{};
//
//     if (auto is_success = 
//         consume_and_advance_expecting(token_type::keyword_program)) {
//         return std::unexpected(*is_success);
//     }
//
//     TRY(result.name, parse_identifier());
//
//     if (m_lexer.lex_next_token().m_type != token_type::l_paren) {
//         result.externals = std::nullopt;
//         return result;
//     }
//     m_lexer.advance_lexer();
//
//     // TRY(
//     //     result.externals, 
//     //     parse_group_of_symbols(
//     //         *this, 
//     //         &parser::parse_identifier, 
//     //         {token_type::r_paren}, 
//     //         token_type::comma
//     //     )
//     // );
//
//     return result;
// }
//
// // block = label-declaration-part constant-definition-part
// //         type-definition-part variable-declaration-part
// //         procedure-and-function-declaration-part statement-part
// auto parser::parse_block()
//     -> parse_result<handle<ast::block>> {
//     auto result = ast::block{};
//     // m_context.initialize_scope();
//     //
//     // if (current_token().type() == token_type::keyword_label) {
//     //     consume_and_advance();
//     //     auto labels = ast::label_declaration{};
//     //     TRY(labels.labels, parse_group_of_symbols(
//     //         *this,
//     //         &parser::parse_identifier,
//     //         {token_type::semicolon}, 
//     //         token_type::comma
//     //     ));
//     //     result.label_declaration_part = std::move(labels);
//     // }
//     //
//     // if (current_token().type() == token_type::keyword_type) {
//     //     
//     // }
//     //
//     // m_context.finalize_scope();
//
//     return result;
//     
// }
//
// /// CONSTANT DECLARATION PARSING 
//
// auto parser::parse_constant()
//     -> parse_result<handle<ast::constant>> {
//     // if sign is specified then constant is treated as a signed number_integer
//     auto result = handle<ast::constant>();
//     auto is_signed = 
//         current_token().type() == token_type::minus;
//     // if (is_signed) consume_and_advance();
//     // 
//     // auto const constant_token = current_token();
//     // // FUCK IT 
//     // // TODO: proper check for number validity
//     // if (constant_token.type() == token_type::number_integer) {
//     //     auto num_view = constant_token.view();
//     //     auto num = unsigned();
//     //     std::from_chars(num_view.data(), num_view.data() + num_view.size(), num);
//     //     if (is_signed) {
//     //         result = -static_cast<int>(num);
//     //     } else {
//     //         result = static_cast<int>(num);
//     //     }
//     // } else if (constant_token.type() == token_type::number_real) {
//     //     auto num_view = constant_token.view();
//     //     auto num = double();
//     //     // I am losing my fucking mind
//     //     // clang doesn't support fp from_chars 
//     //     // IT IS IN GTEH FUCKIGN C++17. 
//     //     // I had to pull boost in (frankly because I am too lazy to search 
//     //     // for the alternatives but still...) 
//     //     boost::charconv::from_chars(
//     //         num_view.data(), num_view.data() + num_view.size(), 
//     //         num, boost::charconv::chars_format::general
//     //     );
//     //     result = is_signed ? -num : num;
//     // } else if (constant_token.type() == token_type::identifier) {
//     //     auto origin_const_exp = m_context.lookup_constant(constant_token.view().base());
//     //     if (not origin_const_exp.has_value()) {
//     //         m_diagnostics.push_back(origin_const_exp.error());
//     //     }
//     //     auto origin_const = m_context.lookup(*origin_const_exp);
//     //     std::visit(overloaded{
//     //         [&](sym::poison_t const&) { result.poison(); },
//     //         [&](auto const& val) { result = val; }
//     //     }, origin_const);
//     // } else if (constant_token.type() == token_type::literal) {
//     //     result = constant_token
//     //         .view()
//     //         .subview(1, constant_token.view().size() - 2)
//     //         .base();
//     // }
//     //
//     return result;
// }
//
/// TYPE DECLARATION PARSING

auto parser::parse_type_definition(semantic_context& ctx)
    -> parse_result<ast::type_declaration> {
    auto identifier_exp = parse_identifier(ctx);
    if (not identifier_exp.has_value()) {
        return std::unexpected(identifier_exp.error());
    }
    auto err = consume_and_advance_expecting(token_type::equal);
    if (not err.has_value()) {
        return std::unexpected(err.error());
    }
    auto type_exp = parse_type(ctx);
    if (not type_exp.has_value()) {
        return std::unexpected(type_exp.error());
    }
    
    return ast::type_declaration{
        .name = std::move(identifier_exp.value()),
        .type = std::move(type_exp.value()),
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
            // return parse_record_type()
            //     .transform(construct<ast::type>);
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
        return std::unexpected(succ.error());
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
    auto begin = parse_constant(ctx);
    if (not begin.has_value()) {
        return std::unexpected(begin.error());
    }
    if (auto succ = consume_and_advance_expecting(token_type::dotdot);
        not succ.has_value()) {
        return std::unexpected(succ.error());
    }
    auto end   = parse_constant(ctx);
    if (not end.has_value()) {
        return std::unexpected(parse_error());
    }
    return ast::subrange_type{
        .begin = *begin,
        .end   = *end,
    };
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
     
    // parse maybe packed , array, [
    if (auto succ = consume_and_advance_expecting(token_type::keyword_array);
        not succ.has_value()) {
        return std::unexpected(succ.error());
    }
    if (auto succ = consume_and_advance_expecting(token_type::l_square);
        not succ.has_value()) {
        return std::unexpected(succ.error());
    }
        
    auto indices = parse_many(&parser::parse_type, ctx, parse_parameters{
        .separator = token_type::comma,
        .success_terminators = {token_type::r_square},
        .hazard_terminators  = {
            token_type::keyword_of, token_type::semicolon,
            token_type::keyword_end, token_type::r_paren,
        },
    }); 

    if (auto succ = consume_and_advance_expecting(token_type::r_square);
        not succ.has_value()) {
        return std::unexpected(succ.error());
    }
    if (auto succ = consume_and_advance_expecting(token_type::keyword_of);
        not succ.has_value()) {
        return std::unexpected(succ.error());
    }

    auto component_type = parse_type(ctx);

    return ast::array_type{
        .index_types = std::move(indices),
        .component_type = std::move(component_type)
            .transform(construct<ast::handle<ast::type>>)
            .value_or(poison_pill),
    };
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
        return std::unexpected(success.error());
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
    if (auto success = consume_and_advance_expecting(token_type::keyword_function);
        not success.has_value()) {
        return std::unexpected(success.error());
    }
    if (auto success = consume_and_advance_expecting(token_type::l_paren);
        not success.has_value()) {
        return std::unexpected(success.error());
    }
    auto argument_list = parse_many(
        &parser::parse_argument, ctx,
        parse_parameters{
            .separator = token_type::comma,
            .success_terminators = {token_type::r_paren},
            .hazard_terminators  = {}
        }
    );
    if (auto success = consume_and_advance_expecting(token_type::r_paren);
        not success.has_value()) {
        return std::unexpected(success.error());
    }
    if (auto success = consume_and_advance_expecting(token_type::colon);
        not success.has_value()) {
        return std::unexpected(success.error());
    }
    
    auto return_type = parse_type(ctx)
        .transform_error(LIFT_MEMBER(push_error))
        .transform(construct<ast::handle<ast::type>>)
        .value_or(poison_pill);

    return ast::function_type{
        .return_type = std::move(return_type),
        .arguments = std::move(argument_list),
    };
}

auto parser::parse_fixed_part(semantic_context const&)
    -> parse_result<ast::fixed_part> {
    return ast::fixed_part{

    };
}
// auto parser::parse_record_type()
//     -> parse_result<handle<ast::record_type>> {
//     auto result = handle<ast::record_type>();
//
//     TRY_OPT(consume_and_advance_expecting(token_type::keyword_record));
//     TRY(result, parse_field_list());
//     TRY_OPT(consume_and_advance_expecting(token_type::keyword_end));
//
//     return result;
// }
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
    return consume_and_advance_expecting(token_type::keyword_set)
        .and_then([this]() { return consume_and_advance_expecting(token_type::keyword_of); })
        .and_then([&]() { return parse_type(ctx); })
        .transform([](auto&& type) { return ast::set_type{ .base = std::move(type) }; });
}

auto parser::parse_file_type(semantic_context const& ctx)
    -> parse_result<ast::file_type> {
    return consume_and_advance_expecting(token_type::keyword_file)
        .and_then([this]() { return consume_and_advance_expecting(token_type::keyword_of); })
        .and_then([&]() { return parse_type(ctx); })
        .transform([](auto&& type) { return ast::file_type{ .component_type = std::move(type) }; });
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
