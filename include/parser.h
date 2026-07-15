#ifndef PARSER_H
#define PARSER_H

#include "tokens.h"
#include "context.h"

/* Buffer para armazenar tokens */
typedef struct {
    Token *tokens;
    int size;
    int capacity;

    /* Array separado para valores numéricos (cache-friendly) */
    double *values;
    int values_size;
    int values_capacity;
} TokenBuffer;

/* Estados de parsing */
typedef enum {
    PARSER_OK = 0,
    PARSER_UNKNOWN_IDENTIFIER = 1,  /* nome que não é variável, função nem constante conhecida */
    PARSER_SYNTAX_ERROR = 2,
    PARSER_ARITY_ERROR = 3,         /* número de argumentos não bate com a aridade da função */
    PARSER_MEMORY_ERROR = 4
} ParserError;

/* Tokeniza `expr` de acordo com `ctx` (locale, variáveis, funções, constantes).
 * Se `error_position` não for NULL, recebe o deslocamento em bytes de `expr`
 * onde o erro foi detectado (ou -1 se não aplicável). */
ParserError parser_tokenize(const AbacoContext *ctx, const char *expr, TokenBuffer *output, int *error_position);

/* Converte tokens infixos para RPN (Shunting-Yard). `ctx` é usado para validar
 * a aridade das chamadas de função. */
ParserError parser_to_rpn(const AbacoContext *ctx, TokenBuffer *tokens, TokenBuffer *rpn);

/* Funções auxiliares */
void parser_init_buffer(TokenBuffer *buf);
void parser_free_buffer(TokenBuffer *buf);
int parser_add_token(TokenBuffer *buf, Token token);

#endif /* PARSER_H */
