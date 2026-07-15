#include <stdio.h>
#include <string.h>
#include "parser.h"
#include "evaluator.h"
#include "debug.h"
#include "assert.h"

static const char *const VARIABLES[] = { "x" };

/* Função helper para testar uma expressão. `expected_rpn_err` cobre erros que só
 * aparecem na conversão para RPN (ex.: PARSER_ARITY_ERROR); nos demais casos é
 * igual a `expected_tokenize_err`. */
static void test_expression_ex(AbacoContext *ctx, const char *expr,
                                ParserError expected_tokenize_err, ParserError expected_rpn_err,
                                EvalError expected_eval_err) {
    printf("\n========================================\n");
    printf("Testando: \"%s\"\n", expr);
    printf("========================================\n");

    TokenBuffer tokens;
    TokenBuffer rpn;

    /* Tokeniza */
    ParserError err = parser_tokenize(ctx, expr, &tokens, NULL);
    assert(err == expected_tokenize_err);
    if (err != PARSER_OK) {
        printf("ERRO: ");
        switch (err) {
            case PARSER_UNKNOWN_IDENTIFIER:
                printf("Identificador desconhecido\n");
                break;
            case PARSER_SYNTAX_ERROR:
                printf("Erro de sintaxe\n");
                break;
            case PARSER_ARITY_ERROR:
                printf("Número de argumentos incorreto\n");
                break;
            case PARSER_MEMORY_ERROR:
                printf("Erro de memória\n");
                break;
            default:
                printf("Erro desconhecido (%d)\n", err);
        }
        return;
    }

    printf("✓ Tokenização OK\n");
    debug_print_tokens(&tokens);

    /* Bytecode compactado */
    debug_print_bytecode(&tokens);

    /* Converte para RPN */
    err = parser_to_rpn(ctx, &tokens, &rpn);
    assert(err == expected_rpn_err);
    if (err != PARSER_OK) {
        printf("ERRO na conversão para RPN: %d\n", err);
        parser_free_buffer(&tokens);
        return;
    }

    printf("✓ Conversão para RPN OK\n");
    debug_print_tokens(&rpn);

    /* Avalia com valor de teste */
    double test_value = 1.0;  /* x = 1 */
    printf("\n--- AVALIAÇÃO (x = %.2f) ---\n", test_value);

    EvalResult eval = evaluator_eval_rpn(ctx, &rpn, &test_value);
    assert(eval.error == expected_eval_err);

    if (eval.error == EVAL_OK) {
        printf("✓ Resultado: %.6g\n", eval.value);

    } else {
        printf("✗ Erro de avaliação: ");
        switch (eval.error) {
            case EVAL_STACK_ERROR:
                printf("Erro na pilha (expressão mal-formada)\n");
                break;
            case EVAL_DIVISION_BY_ZERO:
                printf("Divisão por zero\n");
                break;
            case EVAL_DOMAIN_ERROR:
                printf("Domínio inválido\n");
                break;
            case EVAL_MATH_ERROR:
                printf("Erro matemático (overflow/NaN)\n");
                break;
            default:
                printf("Erro desconhecido\n");
        }
    }

    parser_free_buffer(&rpn);
    parser_free_buffer(&tokens);
}

/* Atalho para o caso comum: o mesmo erro esperado (ou PARSER_OK) nas duas fases. */
static void test_expression(AbacoContext *ctx, const char *expr, ParserError expected_err, EvalError expected_eval_err) {
    test_expression_ex(ctx, expr, expected_err, expected_err, expected_eval_err);
}

int main(void) {
    printf("╔═══════════════════════════════════════════════════════════╗\n");
    printf("║     MULTICURVAS - Parser de Expressões Matemáticas        ║\n");
    printf("║               Fase 1: Tokenizador + RPN                   ║\n");
    printf("╚═══════════════════════════════════════════════════════════╝\n");

    AbacoContext ctx;
    abaco_context_init(&ctx, VARIABLES, 1);

    /* TESTE 1: Com locale POINT (padrão) */
    printf("\n▶▶▶ TESTE COM LOCALE POINT (ponto decimal) ▶▶▶\n");
    ctx.locale = LOCALE_POINT;

    test_expression(&ctx, "sin(x)*2+x", PARSER_OK, EVAL_OK);
    test_expression(&ctx, "9*(x-pi/2)", PARSER_OK, EVAL_OK);
    test_expression(&ctx, "2*e^(-x/2)", PARSER_OK, EVAL_OK);
    test_expression(&ctx, "3.14159", PARSER_OK, EVAL_OK);
    test_expression(&ctx, "2.5*x+1.75", PARSER_OK, EVAL_OK);
    test_expression(&ctx, "0.5^2", PARSER_OK, EVAL_OK);

    /* TESTE 2: Com locale COMMA */
    printf("\n\n▶▶▶ TESTE COM LOCALE COMMA (vírgula decimal) ▶▶▶\n");
    ctx.locale = LOCALE_COMMA;

    test_expression(&ctx, "sin(x)*2+x", PARSER_OK, EVAL_OK);
    test_expression(&ctx, "9*(x-pi/2)", PARSER_OK, EVAL_OK);
    test_expression(&ctx, "2*e^(-x/2)", PARSER_OK, EVAL_OK);
    test_expression(&ctx, "3,14159", PARSER_OK, EVAL_OK);
    test_expression(&ctx, "2,5*x+1,75", PARSER_OK, EVAL_OK);
    test_expression(&ctx, "0,5^2", PARSER_OK, EVAL_OK);

    /* TESTE 3: Erros diversos */
    printf("\n\n▶▶▶ TESTES COM ERROS ▶▶▶\n");
    ctx.locale = LOCALE_POINT;

    test_expression(&ctx, "cossecante(x)", PARSER_UNKNOWN_IDENTIFIER, EVAL_OK);   /* Função desconhecida */
    test_expression(&ctx, "y + 1", PARSER_UNKNOWN_IDENTIFIER, EVAL_OK);           /* Variável não registrada no contexto */
    test_expression(&ctx, "sin(x))", PARSER_SYNTAX_ERROR, EVAL_OK);               /* Parênteses desbalanceados */
    test_expression(&ctx, "pi + e", PARSER_OK, EVAL_OK);                         /* Constantes */
    test_expression(&ctx, "1/0", PARSER_OK, EVAL_DIVISION_BY_ZERO);              /* Divisão por zero */
    test_expression(&ctx, "sqrt(-1)", PARSER_OK, EVAL_DOMAIN_ERROR);             /* Domínio inválido */
    test_expression(&ctx, "log(0)", PARSER_OK, EVAL_DOMAIN_ERROR);               /* Log de zero */
    test_expression_ex(&ctx, "min(1,2,3)", PARSER_OK, PARSER_ARITY_ERROR, EVAL_OK); /* Aridade incorreta: tokeniza OK, falha no RPN */

    /* TESTE 4: Novas funções */
    printf("\n\n▶▶▶ TESTES COM NOVAS FUNÇÕES ▶▶▶\n");

    test_expression(&ctx, "log(e)", PARSER_OK, EVAL_OK);                  /* log(e) = 1 */
    test_expression(&ctx, "log10(100)", PARSER_OK, EVAL_OK);              /* log10(100) = 2 */
    test_expression(&ctx, "sinh(0)", PARSER_OK, EVAL_OK);                 /* sinh(0) = 0 */
    test_expression(&ctx, "asin(0.5)", PARSER_OK, EVAL_OK);               /* asin(0.5) ≈ 0.523599 (π/6) */
    test_expression(&ctx, "ceil(2.3)", PARSER_OK, EVAL_OK);               /* ceil(2.3) = 3 */
    test_expression(&ctx, "floor(2.7)", PARSER_OK, EVAL_OK);              /* floor(2.7) = 2 */
    test_expression(&ctx, "frac(3.14)", PARSER_OK, EVAL_OK);              /* frac(3.14) = 0.14 */

    /* TESTE 5: Funções multi-argumento (novidade da generalização) */
    printf("\n\n▶▶▶ TESTES COM FUNÇÕES MULTI-ARGUMENTO ▶▶▶\n");

    test_expression(&ctx, "atan2(1,1)", PARSER_OK, EVAL_OK);              /* atan2(1,1) = pi/4 */
    test_expression(&ctx, "pow(2,10)", PARSER_OK, EVAL_OK);               /* pow(2,10) = 1024 */
    test_expression(&ctx, "min(3,5)", PARSER_OK, EVAL_OK);                /* min(3,5) = 3 */
    test_expression(&ctx, "max(3,5)", PARSER_OK, EVAL_OK);                /* max(3,5) = 5 */

    printf("\n╔═══════════════════════════════════════════════════════════╗\n");
    printf("║                    Testes Completos                       ║\n");
    printf("╚═══════════════════════════════════════════════════════════╝\n");

    return 0;
}
