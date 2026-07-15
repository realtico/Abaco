#ifndef ABACO_CONTEXT_H
#define ABACO_CONTEXT_H

#include "tokens.h"

/* Aridade máxima suportada para funções registradas. */
#define ABACO_MAX_ARITY 2

typedef enum {
    ABACO_FN_OK = 0,
    ABACO_FN_DOMAIN_ERROR = 1   /* domínio inválido (ex.: sqrt de negativo) */
} AbacoFnStatus;

typedef struct {
    AbacoFnStatus status;
    double value;               /* válido apenas se status == ABACO_FN_OK */
} AbacoFnResult;

/* args[0..arity-1] na ordem em que aparecem na chamada, ex. atan2(y,x) -> args={y,x} */
typedef AbacoFnResult (*AbacoFn)(const double *args);

typedef struct {
    const char *name;
    int arity;          /* 1..ABACO_MAX_ARITY */
    AbacoFn fn;
} AbacoFunction;

typedef struct {
    const char *name;
    double value;
} AbacoConstant;

/* Contexto de parsing/avaliação: define quais variáveis, funções e constantes
 * uma expressão pode referenciar, e o locale usado para números.
 *
 * As tabelas (variables/functions/constants) são apenas referenciadas, não
 * copiadas - devem permanecer válidas enquanto o contexto for usado. */
typedef struct {
    LocaleConfig locale;

    const char *const *variables;
    int variable_count;

    const AbacoFunction *functions;
    int function_count;

    const AbacoConstant *constants;
    int constant_count;
} AbacoContext;

/* Tabelas prontas com as funções/constantes matemáticas padrão (sin, cos, sqrt,
 * log, atan2, pi, e, ...). Ver src/context.c para a lista completa. */
extern const AbacoFunction ABACO_BUILTIN_FUNCTIONS[];
extern const int ABACO_BUILTIN_FUNCTION_COUNT;
extern const AbacoConstant ABACO_BUILTIN_CONSTANTS[];
extern const int ABACO_BUILTIN_CONSTANT_COUNT;

/* Inicializa `ctx` com locale POINT e as tabelas builtin de funções/constantes.
 * `variables` deve permanecer válido enquanto o contexto for usado.
 * Para customizar funções/constantes, sobrescreva ctx->functions/constants
 * (e respectivos *_count) depois de chamar esta função. */
void abaco_context_init(AbacoContext *ctx, const char *const *variables, int variable_count);

#endif /* ABACO_CONTEXT_H */
