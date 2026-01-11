#ifndef TOKENS_H
#define TOKENS_H

#include <stdint.h>

/* Configuração de locale - marca decimal */
typedef enum {
    LOCALE_POINT,      /* 3.14 (padrão C/EN) */
    LOCALE_COMMA       /* 3,14 (PT-BR, FR, DE) */
} LocaleConfig;

/* Token types - valores >= 128 são especiais (funções, constantes, variáveis) */
typedef enum {
    /* Operadores (ASCII) */
    TOKEN_PLUS       = '+',
    TOKEN_MINUS      = '-',
    TOKEN_MULT       = '*',
    TOKEN_DIV        = '/',
    TOKEN_POW        = '^',
    TOKEN_LPAREN     = '(',
    TOKEN_RPAREN     = ')',
    
    /* Especiais >= 128 */
    TOKEN_NUMBER     = 128,
    
    /* Variáveis: range 129-138 (10 slots para customização) */
    TOKEN_VARIABLE_X = 129,
    TOKEN_VARIABLE_THETA = 130,
    TOKEN_VARIABLE_T = 131,
    /* Slots 132-138 disponíveis para novas variáveis */
    
    /* Constantes: range 140-159 (20 slots para customização) */
    TOKEN_CONST_PI   = 140,
    TOKEN_CONST_E    = 141,
    /* Slots 142-159 disponíveis para novas constantes */
    
    /* Funções: range 160-199 (40 slots para customização) */
    TOKEN_SIN        = 160,
    TOKEN_COS        = 161,
    TOKEN_TAN        = 162,
    TOKEN_ABS        = 163,
    TOKEN_SQRT       = 164,
    /* Slots 165-199 disponíveis para novas funções */
    /* Slots 165-199 disponíveis para novas funções */
    
    TOKEN_END        = 255,  /* Fim da expressão */
    TOKEN_ERROR      = 256   /* Erro de parsing */
} TokenType;

/* Ranges para extensibilidade (usados em is_variable, is_constant, is_function) */
#define TOKEN_VARIABLE_START  129
#define TOKEN_VARIABLE_END    138

#define TOKEN_CONST_START     140
#define TOKEN_CONST_END       159

#define TOKEN_FUNCTION_START  160
#define TOKEN_FUNCTION_END    199

typedef struct {
    TokenType type;
    double value;      /* Para TOKEN_NUMBER */
} Token;

#endif /* TOKENS_H */
