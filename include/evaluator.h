#ifndef EVALUATOR_H
#define EVALUATOR_H

#include "parser.h"

/* Erros de avaliação */
typedef enum {
    EVAL_OK = 0,
    EVAL_STACK_ERROR,           /* Pilha vazia ou com múltiplos valores no final */
    EVAL_DIVISION_BY_ZERO,      /* Divisão por zero (permite estratégias de limite/stencil) */
    EVAL_DOMAIN_ERROR,          /* Domínio inválido (sqrt negativo, log negativo, etc.) */
    EVAL_MATH_ERROR             /* Outros erros matemáticos (overflow, underflow, NaN) */
} EvalError;

/* Resultado da avaliação */
typedef struct {
    EvalError error;
    double value;               /* Valor calculado (válido apenas se error == EVAL_OK) */
} EvalResult;

/* Avalia expressão em RPN. `variable_values[i]` fornece o valor de `ctx->variables[i]`
 * (o array deve ter pelo menos `ctx->variable_count` posições). */
EvalResult evaluator_eval_rpn(const AbacoContext *ctx, const TokenBuffer *rpn, const double *variable_values);

#endif /* EVALUATOR_H */
