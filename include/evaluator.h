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

/* Avalia expressão em RPN com valor para a variável */
EvalResult evaluator_eval_rpn(const TokenBuffer *rpn, double var_value);

#endif /* EVALUATOR_H */
