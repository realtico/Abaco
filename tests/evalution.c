#include <stdio.h>
#include <string.h>
#include "parser.h"
#include "evaluator.h"
#include "abaco_test.h"

static const char *const VARIABLES[] = { "x" };

/* NAN sentinel: usado quando o valor esperado não importa (a expressão deve falhar
 * antes de produzir um resultado, ou o chamador só quer checar o código de erro). */
#define DONT_CHECK_VALUE 0.0

/* Função helper para testar uma expressão. `expected_rpn_err` cobre erros que só
 * aparecem na conversão para RPN (ex.: PARSER_ARITY_ERROR); nos demais casos é
 * igual a `expected_tokenize_err`. Se `expected_eval_err == EVAL_OK`, o resultado
 * numérico é comparado com `expected_value` (tolerância `1e-6`). */
static void test_expression_ex(AbacoContext *ctx, const char *expr,
                                ParserError expected_tokenize_err, ParserError expected_rpn_err,
                                EvalError expected_eval_err, double expected_value) {
    printf("\n--- Testando: \"%s\" ---\n", expr);

    TokenBuffer tokens;
    TokenBuffer rpn;

    /* Tokeniza */
    ParserError err = parser_tokenize(ctx, expr, &tokens, NULL);
    ABACO_ASSERT(err == expected_tokenize_err);
    if (err != PARSER_OK) {
        printf("ERRO na tokenização: %d\n", err);
        return;
    }

    /* Converte para RPN */
    err = parser_to_rpn(ctx, &tokens, &rpn);
    ABACO_ASSERT(err == expected_rpn_err);
    if (err != PARSER_OK) {
        printf("ERRO na conversão para RPN: %d\n", err);
        parser_free_buffer(&tokens);
        return;
    }

    /* Avalia com valor de teste */
    double test_value = 1.0;  /* x = 1 */
    EvalResult eval = evaluator_eval_rpn(ctx, &rpn, &test_value);
    ABACO_ASSERT(eval.error == expected_eval_err);

    if (eval.error == EVAL_OK) {
        printf("Resultado: %.6g\n", eval.value);
        if (expected_eval_err == EVAL_OK) {
            ABACO_ASSERT_NEAR(eval.value, expected_value, 1e-6);
        }
    } else {
        printf("Erro de avaliação: %d\n", eval.error);
    }

    parser_free_buffer(&rpn);
    parser_free_buffer(&tokens);
}

/* Atalho para o caso comum: mesmo erro de parsing nas duas fases + checagem de valor. */
static void test_expression(AbacoContext *ctx, const char *expr, double expected_value) {
    test_expression_ex(ctx, expr, PARSER_OK, PARSER_OK, EVAL_OK, expected_value);
}

/* Atalho para expressões que devem falhar no parsing (tokenização ou RPN). */
static void test_parser_error(AbacoContext *ctx, const char *expr, ParserError expected_tokenize_err, ParserError expected_rpn_err) {
    test_expression_ex(ctx, expr, expected_tokenize_err, expected_rpn_err, EVAL_OK, DONT_CHECK_VALUE);
}

/* Atalho para expressões que devem falhar na avaliação (x = 1). */
static void test_eval_error(AbacoContext *ctx, const char *expr, EvalError expected_eval_err) {
    test_expression_ex(ctx, expr, PARSER_OK, PARSER_OK, expected_eval_err, DONT_CHECK_VALUE);
}

int main(void) {
    AbacoContext ctx;
    abaco_context_init(&ctx, VARIABLES, 1);

    /* Com locale POINT (padrão), x = 1 */
    printf("\n▶ Locale POINT (ponto decimal)\n");
    ctx.locale = LOCALE_POINT;

    test_expression(&ctx, "sin(x)*2+x", 2.682941969);
    test_expression(&ctx, "9*(x-pi/2)", -5.137167185);
    test_expression(&ctx, "2*e^(-x/2)", 1.213061319);
    test_expression(&ctx, "3.14159", 3.14159);
    test_expression(&ctx, "2.5*x+1.75", 4.25);
    test_expression(&ctx, "0.5^2", 0.25);

    /* Com locale COMMA, mesmas expressões e resultados */
    printf("\n▶ Locale COMMA (vírgula decimal)\n");
    ctx.locale = LOCALE_COMMA;

    test_expression(&ctx, "sin(x)*2+x", 2.682941969);
    test_expression(&ctx, "9*(x-pi/2)", -5.137167185);
    test_expression(&ctx, "2*e^(-x/2)", 1.213061319);
    test_expression(&ctx, "3,14159", 3.14159);
    test_expression(&ctx, "2,5*x+1,75", 4.25);
    test_expression(&ctx, "0,5^2", 0.25);

    /* Erros diversos */
    printf("\n▶ Erros\n");
    ctx.locale = LOCALE_POINT;

    test_parser_error(&ctx, "cossecante(x)", PARSER_UNKNOWN_IDENTIFIER, PARSER_UNKNOWN_IDENTIFIER); /* Função desconhecida */
    test_parser_error(&ctx, "y + 1", PARSER_UNKNOWN_IDENTIFIER, PARSER_UNKNOWN_IDENTIFIER);         /* Variável não registrada no contexto */
    test_parser_error(&ctx, "sin(x))", PARSER_SYNTAX_ERROR, PARSER_SYNTAX_ERROR);                   /* Parênteses desbalanceados */
    test_expression(&ctx, "pi + e", 5.859874482);                                                   /* Constantes */
    test_eval_error(&ctx, "1/0", EVAL_DIVISION_BY_ZERO);
    test_eval_error(&ctx, "sqrt(-1)", EVAL_DOMAIN_ERROR);
    test_eval_error(&ctx, "log(0)", EVAL_DOMAIN_ERROR);
    test_parser_error(&ctx, "min(1,2,3)", PARSER_OK, PARSER_ARITY_ERROR); /* Tokeniza OK, falha só no RPN (aridade) */

    /* Funções */
    printf("\n▶ Funções\n");

    test_expression(&ctx, "log(e)", 1.0);
    test_expression(&ctx, "log10(100)", 2.0);
    test_expression(&ctx, "sinh(0)", 0.0);
    test_expression(&ctx, "asin(0.5)", 0.5235987756);
    test_expression(&ctx, "ceil(2.3)", 3.0);
    test_expression(&ctx, "floor(2.7)", 2.0);
    test_expression(&ctx, "frac(3.14)", 0.14);

    /* Funções multi-argumento */
    printf("\n▶ Funções multi-argumento\n");

    test_expression(&ctx, "atan2(1,1)", 0.7853981634);
    test_expression(&ctx, "pow(2,10)", 1024.0);
    test_expression(&ctx, "min(3,5)", 3.0);
    test_expression(&ctx, "max(3,5)", 5.0);

    return abaco_test_summary("evalution");
}
