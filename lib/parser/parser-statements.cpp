#include "fed/diagnostics/compile-error.hpp"
#include "fed/diagnostics/internal-error.hpp"
#include "fed/parser/context.hpp"
#include "fed/parser/parse_error.hpp"
#include "fed/parser/parser-helpers.hpp"
#include "fed/parser/parser.hpp"
#include "fed/representations/ast.hpp"
#include "fed/representations/ast/clone.hpp"
#include "fed/representations/ast/forward.hpp"
#include "fed/representations/ast/nodes.hpp"
#include "fed/scanner/token.hpp"
#include "fed/utils/superutil.hpp"
#include <optional>

namespace fed {

auto parser::parse_statement(semantic_context& ctx) 
    -> parse_result<ast::statement> {

    switch (current_token().type()) {
        using enum token_type;
        case keyword_case: {
            return parse_case_statement(ctx)
                .transform(construct<ast::statement>);
        }
        case keyword_if: {
            return parse_if_statement(ctx)
                .transform(construct<ast::statement>);
        }
        case keyword_for: {
            return parse_for_statement(ctx)
                .transform(construct<ast::statement>);
        }
        case keyword_while: {
            return parse_while_statement(ctx)
                .transform(construct<ast::statement>);
        }
        case keyword_repeat: {
            return parse_repeat_statement(ctx)
                .transform(construct<ast::statement>);
        }
        case keyword_begin: {
            return parse_compound_statement(ctx)
                .transform(construct<ast::statement>);
        }
        case keyword_with: {
            return parse_with_statement(ctx)
                .transform(construct<ast::statement>);
        }
        case semicolon: {
            consume_and_advance();
            return ast::empty_statement();
        }
        case identifier: {
            switch (lookahead_token(1).type()) {
                case equal:
                case define: {
                    return parse_assignment_statement(ctx)
                        .transform(construct<ast::statement>);
                }
                case l_paren:  {
                    return parse_procedure_statement(ctx)
                        .transform(construct<ast::statement>);
                }
                default: {
                    return std::unexpected(
                        dummy_error(cursor().where(), "bad statement")
                    );
                }
            }

        }
        default: {
            return std::unexpected(
                dummy_error(cursor().where(), "bad statement")
            );
        }
    }
}

auto parser::parse_case_statement(semantic_context& ctx)
    -> parse_result<ast::case_statement> {
    using enum token_type;
    auto action = [](
        auto index_case,
        auto cases
    ) {
        auto index_case_handle = 
            monad_to_handle(std::move(index_case));
    
        return ast::case_statement{
            .case_index = std::move(index_case_handle),
            .cases = std::move(*cases),
        };
    };
    auto case_action = [](auto constants, auto stmt) {
        return identity_monad(ast::case_t{
            .values = std::move(*constants),
            .action = monad_to_handle(std::move(stmt)),
        });
    };
    auto parse_case_case = make_chain_parse(
        case_action,
        make_many_parse(parse_constant_f, comma),
        colon,
        parse_statement_f
    );

    return chain_parse(
        ctx, action, 
        keyword_case, parse_expression_f, keyword_of,
        make_many_parse(parse_case_case, empty),
        keyword_end
    );
}

auto parser::parse_if_statement(semantic_context& ctx)
    -> parse_result<ast::if_statement> {
    using enum token_type;
    auto action = [](auto condition, auto then, auto else_) {
        return ast::if_statement{
            .condition = monad_to_handle(std::move(condition)),
            .then_case = monad_to_handle(std::move(then)),
            .else_case = std::move(*else_),
        };
    };
    using else_t = 
        ast::maybe<ast::handle<ast::statement>>
    ;
    auto else_parse = [](
        parser& parser, 
        semantic_context& ctx
    ) -> identity_monad<else_t> {
        if (parser.current_token_is(equal_to(keyword_else))) {
            parser.consume_and_advance();
            return else_t(monad_to_handle(
                parser.breach_monitor_parse(ctx, parse_statement_f)
            ));
        } else { 
            return else_t(std::nullopt);
        }
    };

    return chain_parse(
        ctx, action,
        keyword_if, parse_expression_f,
        keyword_then, parse_statement_f,
        else_parse
    );
}

auto parser::parse_for_statement(semantic_context& ctx)
    -> parse_result<ast::for_statement> {
    using enum token_type;
    using order_t = ast::for_statement::iteration_t;
    struct for_head {
        ast::handle<ast::identifier> var;
        ast::handle<ast::expression> init;
        ast::handle<ast::expression> end;
        order_t order;
    };
    auto parse_order = [](parser& parser, semantic_context& ctx) 
        -> parse_result<order_t> {
        switch (parser.current_token().type()) {
            case keyword_to: {
                parser.consume_and_advance();
                return order_t::ascending;
            }
            case keyword_downto: {
                parser.consume_and_advance();
                return order_t::descending;
            }
            default: return std::unexpected(
                dummy_error(
                    parser.cursor().where(),
                    "invalid order specifier"
                )
            );
        }
    };
    auto head_action = [](
        auto idnt, auto init, auto order, auto end
    ) {
        return for_head{
            .var   = monad_to_handle(std::move(idnt)),
            .init  = monad_to_handle(std::move(init)),
            .end   = monad_to_handle(std::move(end)),
            .order = order.value_or(order_t::bad)
        };

    };
    // for identifier := expr to/downto expr 
    auto head = chain_parse(
        ctx, head_action,
        keyword_for, 
        parse_identifier_f, define, parse_expression_f,
        parse_order, parse_expression_f
    );
    auto for_ctx = ctx.make_local();
    // TODO: type match init and end 
    auto id = for_ctx.add_variable(ast::variable_declaration{
        .name = head.var.value_or("##bogus name"),
        .type = ast::materialize(ctx.get_expression_type(head.init)),
    }).sploink(diagnostics());

    auto final_act = [&](auto stmt) {
        return ast::for_statement{
            .ctx = std::move(for_ctx),
            .control_variable = id,
            .iteration = head.order,
            .initial_value = std::move(head.init),
            .final_value = std::move(head.end),
            .action = monad_to_handle(std::move(stmt)),
        };
    };

    return chain_parse(
        for_ctx, final_act,
        keyword_do, parse_statement_f
    );
}

auto parser::parse_while_statement(semantic_context& ctx)
    -> parse_result<ast::while_statement> {
    using enum token_type;
    auto action = [](auto cond, auto stmt) {
        return ast::while_statement{
            .condition = monad_to_handle(std::move(cond)),
            .action    = monad_to_handle(std::move(stmt)),
        };
    };

    return chain_parse(
        ctx, action,
        keyword_while, parse_expression_f, 
        keyword_do, parse_statement_f
    );
}
auto parser::parse_repeat_statement(semantic_context& ctx)
    -> parse_result<ast::repeat_statement> {
    using enum token_type;
    auto action = [](auto stmt, auto cond) {
        return ast::repeat_statement{
            .condition = monad_to_handle(std::move(cond)),
            .action    = monad_to_handle(std::move(stmt)),
        };
    };

    return chain_parse(
        ctx, action,
        keyword_repeat, parse_statement_f,
        keyword_until, parse_expression_f, semicolon
    );
}

auto parser::parse_compound_statement(semantic_context& ctx)
    -> parse_result<ast::compound_statement> {
    using enum token_type;
    auto action = [](auto seq) {
        return ast::compound_statement{
            .statements = std::move(*seq)
        };
    };

    return chain_parse(
        ctx, action,
        keyword_begin, 
        make_many_parse(parse_statement_f, empty),
        keyword_end
    );
}

auto parser::parse_with_statement(semantic_context& ctx)
    -> parse_result<ast::with_statement> {
    using enum token_type;
    auto with_ctx = ctx.make_local();
    auto record_names = *chain_parse(
        ctx, id,
        keyword_with,
        make_many_parse(parse_identifier_f, comma),
        keyword_do 
    );
    auto record_variables = ast::group<ast::variable_id>();
    for (auto&& record_name : record_names) {
        ast::variable_id id = record_name.and_then(cure(
            [&](ast::identifier const& name) {
                return ctx
                    .try_get_variable_id(name)
                    .value_or(ast::variable_id::poison);
            }
        ));

        id = ctx.type_from_id(id)
            .and_then(cure([&](ast::type const& type) {
                if (holds_alternative<ast::record_type>(type)) {
                    return id;
                } else {
                    diagnostics().push_back(
                        dummy_error(
                            cursor().where(), 
                            "not a record variable"
                        )
                    );
                    return ast::variable_id::poison;
                }
            }));
        
        record_variables.push_back(id);
    }
    auto _ = with_ctx.expose_records_fields(record_variables);
    auto stmt = monad_to_handle(
        breach_monitor_parse(ctx, parse_statement_f)
    );

    return ast::with_statement{
        .ctx = std::move(with_ctx),
        .variables = std::move(record_variables),
        .action = std::move(stmt),
    };
}

auto parser::parse_assignment_statement(semantic_context& ctx)
    -> parse_result<ast::assignment_statement> {
    throw internal_error("tbi");
    using enum token_type;
    auto action = [](auto asgn, auto expr) {
        return ast::assignment_statement{
            .variable = ast::variable_id::poison,
            .value = monad_to_handle(std::move(expr)),
        };
    };

    return chain_parse(
        ctx, action, 
        parse_identifier_f, define, parse_expression_f, semicolon
    );
}

auto parser::parse_procedure_statement(semantic_context& ctx)
    -> parse_result<ast::procedure_statement> {
    throw internal_error("tbi");
    return ast::procedure_statement{
        .procedure = ast::procedure_id::poison,
        .call_args = {},
    };
}


}


