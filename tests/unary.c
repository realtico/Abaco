#include <stdio.h>
#include "parser.h"
#include "evaluator.h"
#include "debug.h"

static const char *const VARIABLES[] = { "x" };

int main(void) {
    AbacoContext ctx;
    abaco_context_init(&ctx, VARIABLES, 1);
    ctx.locale = LOCALE_POINT;

    const char *tests[] = {
        "2*e^(-x/2)",
        "-x",
        "-x+3",
        "2*(-x)",
        "sin(-x)",
        "-2*x",
        "x+-3",
        "(-x)^2",
        "+x",
        "x+ +3",
        "2*(+x)"
        ,"--x",
        "-+x",
        "+-x",
        "---x"
    };

    int test_count = sizeof(tests) / sizeof(tests[0]);
    for (int t = 0; t < test_count; t++) {
        const char *expr = tests[t];
        printf("\n========================================\n");
        printf("Testando: %s\n", expr);
        printf("========================================\n");

        TokenBuffer tokens;
        ParserError err = parser_tokenize(&ctx, expr, &tokens, NULL);

        if (err != PARSER_OK) {
            printf("✗ Erro no parsing: %d\n", err);
            continue;
        }

        TokenBuffer rpn;
        err = parser_to_rpn(&ctx, &tokens, &rpn);

        if (err != PARSER_OK) {
            printf("✗ Erro na conversão RPN: %d\n", err);
            parser_free_buffer(&tokens);
            continue;
        }


        printf("✓ Parse OK - Testando avaliação:\n");
        double x = 2.0;
        EvalResult result = evaluator_eval_rpn(&ctx, &rpn, &x);
        if (result.error == EVAL_OK) {
            printf("  x=2: %.6f\n", result.value);
        } else {
            printf("  x=2: ERRO %d\n", result.error);
        }

        parser_free_buffer(&tokens);
        parser_free_buffer(&rpn);
    }

    return 0;
}
