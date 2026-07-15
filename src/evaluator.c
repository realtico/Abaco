#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "evaluator.h"

/* Tamanho máximo da pilha de avaliação (otimização de desempenho) */
#define MAX_EVAL_STACK_SIZE 64

/* Aplica operador binário (token operador, operando esquerdo, operando direito) */
static inline EvalResult apply_operator(TokenType type, double left, double right) {
    EvalResult result = {EVAL_OK, 0.0};

    switch (type) {
        case TOKEN_PLUS:
            result.value = left + right;
            break;
        case TOKEN_MINUS:
            result.value = left - right;
            break;
        case TOKEN_MULT:
            result.value = left * right;
            break;
        case TOKEN_DIV:
            /* Erro específico para divisão por zero */
            if (right == 0.0) {
                result.error = EVAL_DIVISION_BY_ZERO;
                return result;
            }
            result.value = left / right;
            break;
        case TOKEN_POW:
            result.value = pow(left, right);
            /* pow pode retornar NaN para casos como (-1)^0.5 */
            if (isnan(result.value)) {
                result.error = EVAL_DOMAIN_ERROR;
                return result;
            }
            break;
        default:
            result.error = EVAL_MATH_ERROR;
            break;
    }

    /* Verifica overflow/underflow */
    if (isnan(result.value) || isinf(result.value)) {
        result.error = EVAL_MATH_ERROR;
    }

    return result;
}

/* Avalia expressão em RPN */
EvalResult evaluator_eval_rpn(const AbacoContext *ctx, const TokenBuffer *rpn, const double *variable_values) {
    EvalResult result = {EVAL_OK, 0.0};

    if (!ctx || !rpn || !rpn->tokens || rpn->size == 0) {
        result.error = EVAL_STACK_ERROR;
        return result;
    }

    /* Pilha estática de valores (sem malloc/free para otimização) */
    double stack[MAX_EVAL_STACK_SIZE];
    int stack_top = -1;

    /* Processa cada token da expressão RPN */
    for (int i = 0; i < rpn->size; i++) {
        Token token = rpn->tokens[i];
        TokenType type = token.type;

        switch (type) {
            case TOKEN_END:
                goto evaluation_end;

            case TOKEN_NUMBER:
                if (stack_top >= MAX_EVAL_STACK_SIZE - 1) {
                    result.error = EVAL_STACK_ERROR;
                    return result;
                }
                stack[++stack_top] = rpn->values[token.value_index];
                break;

            case TOKEN_VARIABLE:
                if (stack_top >= MAX_EVAL_STACK_SIZE - 1) {
                    result.error = EVAL_STACK_ERROR;
                    return result;
                }
                stack[++stack_top] = variable_values[token.value_index];
                break;

            /* Operadores binários */
            case TOKEN_PLUS:
            case TOKEN_MINUS:
            case TOKEN_MULT:
            case TOKEN_DIV:
            case TOKEN_POW:
                if (stack_top < 1) {
                    result.error = EVAL_STACK_ERROR;
                    return result;
                }
                {
                    double right = stack[stack_top--];
                    double left = stack[stack_top--];
                    EvalResult op_result = apply_operator(type, left, right);
                    if (op_result.error != EVAL_OK) return op_result;
                    stack[++stack_top] = op_result.value;
                }
                break;

            /* Operador unário (NEG) */
            case TOKEN_NEG:
                if (stack_top < 0) {
                    result.error = EVAL_STACK_ERROR;
                    return result;
                }
                stack[stack_top] = -stack[stack_top];
                break;

            /* Função registrada em ctx->functions[value_index] */
            case TOKEN_FUNCTION: {
                if (token.value_index >= (uint16_t)ctx->function_count) {
                    result.error = EVAL_MATH_ERROR;
                    return result;
                }
                const AbacoFunction *fn = &ctx->functions[token.value_index];
                if (stack_top < fn->arity - 1) {
                    result.error = EVAL_STACK_ERROR;
                    return result;
                }
                double args[ABACO_MAX_ARITY];
                for (int a = fn->arity - 1; a >= 0; a--) {
                    args[a] = stack[stack_top--];
                }
                AbacoFnResult fn_result = fn->fn(args);
                if (fn_result.status == ABACO_FN_DOMAIN_ERROR) {
                    result.error = EVAL_DOMAIN_ERROR;
                    return result;
                }
                if (isnan(fn_result.value) || isinf(fn_result.value)) {
                    result.error = EVAL_MATH_ERROR;
                    return result;
                }
                stack[++stack_top] = fn_result.value;
                break;
            }

            default:
                /* Token desconhecido */
                result.error = EVAL_MATH_ERROR;
                return result;
        }
    }

evaluation_end: ;

    /* Deve sobrar exatamente 1 valor na pilha */
    if (stack_top != 0) {
        result.error = EVAL_STACK_ERROR;
        return result;
    }

    result.value = stack[0];

    return result;
}
