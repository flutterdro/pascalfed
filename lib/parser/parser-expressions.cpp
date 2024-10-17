#include "fed/parser/parser.hpp"
#include "fed/representations/parse-tree.hpp"
#include "fed/scanner/token.hpp"
#include "fed/utils/superutil.hpp"
#include <fmt/base.h>
#include <fmt/ostream.h>
#include <utility>

#define TRY(dest, exp) \
do {auto exp2 = exp;\
if (not exp2.has_value()) return std::unexpected{std::move(exp2.error())}; \
dest = std::move(exp2.value());} while(false)
#define TRY_OPT(opt) \
if (opt.has_value()) return std::unexpected{std::move(*opt)};


namespace fed {

using ast::handle;
int g_counter = 0;
auto parser::parse_expression()
    -> parse_result<handle<ast::expression>> {
    fmt::println("Called parse_expression()");
    fmt::println("{}: Current cursor on: {}",g_counter++, current_token().view().base());
    if (current_token().type() == token_type::l_paren) {
        consume_and_advance();
        auto lhs = *parse_expression();
        TRY_OPT(consume_and_advance_expecting(token_type::r_paren));
        return parse_expression(std::move(lhs), precedence::lowest);
    }
    fmt::println("{}: Current cursor on: {}",g_counter++, current_token().view().base());
    if (func::any_of(std::array{
        token_type::plus,
        token_type::minus,
        token_type::keyword_not,
    })(current_token().type())) { return parse_unary_expression(); }
    fmt::println("{}: Current cursor on: {}",g_counter++, current_token().view().base());

    auto lhs = *parse_expression_leaf();

    fmt::println("{}: Current cursor on: {}",g_counter++, current_token().view().base());
    return parse_expression(std::move(lhs), precedence::lowest);

}

auto parser::parse_unary_expression()
    -> parse_result<handle<ast::unary_expression>> {
    auto result = handle<ast::unary_expression>();
    auto token = consume_and_advance().type();

    auto token_to_operation = [](token_type type) -> ast::unary_operation {
        switch (type) {
            case token_type::plus:        return ast::unary_operation::identity;  
            case token_type::minus:       return ast::unary_operation::negation;
            case token_type::keyword_not: return ast::unary_operation::logical_negation;
            default: std::unreachable();
        }
    };

    result->operation = token_to_operation(token);

    if (current_token().type() == token_type::l_paren) {
        TRY(result->operand, parse_expression());
    } else {
        TRY(result->operand, parse_expression_leaf());
    }
   
    return result;
}
inline constexpr auto is_binary_operator = [](token_type type) {
    return func::any_of(std::array{
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

auto parser::parse_expression(handle<ast::expression> lhs, precedence::level threshold)
    -> parse_result<handle<ast::expression>> {
    fmt::println("Called parse_expression(_, {})", +threshold);
    
    auto token = current_token().type();
    fmt::println("{}: Current cursor on: {}",g_counter++, current_token().view().base());

    if (not is_binary_operator(token)) return lhs;
    fmt::println("{}: Current cursor on: {}",g_counter++, current_token().view().base());

    if (auto new_threshold = binary_operator_precedence(token);
        new_threshold > threshold) {
        lhs = *parse_expression(std::move(lhs), new_threshold);
    }
    if (not is_binary_operator(current_token().type())) return lhs;
    fmt::println("{}: Current cursor on: {}",g_counter++, current_token().view().base());
    
    
    return parse_rhs(std::move(lhs), threshold);
}


auto parser::parse_rhs(handle<ast::expression> lhs, precedence::level threshold) 
    -> parse_result<handle<ast::binary_expression>> {
    fmt::println("Called parse_rhs(_, {})", +threshold);
    auto result = handle<ast::binary_expression>();
    auto token_to_operation = [](token_type type) {
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

    if (not is_binary_operator(current_token().type())) { 
            fmt::println("mark 8");
        return std::unexpected(parse_error());}
    fmt::println("{}: Current cursor on: {}",g_counter++, current_token().view().base());
    auto operation_token = consume_and_advance().type();
    auto rhs = [&] -> handle<ast::expression> {
        if (current_token().type() == token_type::l_paren) {
            return *parse_expression();
        } else {
            auto rhs = *parse_expression_leaf();
            if (auto tok = current_token().type();
                is_binary_operator(tok) and binary_operator_precedence(tok)  > threshold) {
                return *parse_expression(std::move(rhs), threshold);
            }
            return rhs;
        }
    }();
    fmt::println("{}: Current cursor on: {}",g_counter++, current_token().view().base());
    result = ast::binary_expression{
        .lhs       = std::move(lhs),
        .rhs       = std::move(rhs),
        .operation = token_to_operation(operation_token),
    };

    return result;
}

auto parser::parse_expression_leaf() 
    -> parse_result<handle<ast::expression_leaf>> {
    fmt::println("Called parse_leaf()");
    auto characto = consume_and_advance().view().base();
    fmt::println("{}", characto);
    return handle{ast::expression_leaf{.character = characto[0]}};
}



}
