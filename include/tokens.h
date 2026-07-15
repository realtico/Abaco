#ifndef TOKENS_H
#define TOKENS_H

#include <stdint.h>

/* Configuração de locale - marca decimal */
typedef enum {
    LOCALE_POINT,      /* 3.14 (padrão C/EN) */
    LOCALE_COMMA       /* 3,14 (PT-BR, FR, DE) */
} LocaleConfig;

/* Token types - valores >= 128 são especiais (não-ASCII) */
typedef enum {
    /* Operadores e pontuação (ASCII) */
    TOKEN_PLUS       = '+',
    TOKEN_MINUS      = '-',
    TOKEN_MULT       = '*',
    TOKEN_DIV        = '/',
    TOKEN_POW        = '^',
    TOKEN_LPAREN     = '(',
    TOKEN_RPAREN     = ')',
    TOKEN_COMMA      = ',',

    /* Especiais >= 128. Variáveis e funções são genéricas: a identidade real
     * (nome, aridade, implementação) vive em AbacoContext, não no token. */
    TOKEN_NUMBER     = 128, /* literal numérico (constantes também viram isto ao parsear) */
    TOKEN_VARIABLE   = 129, /* value_index = índice em AbacoContext.variables */
    TOKEN_FUNCTION   = 130, /* value_index = índice em AbacoContext.functions */
    TOKEN_NEG        = 131, /* negação unária prefixa */

    TOKEN_END        = 255,  /* Fim da expressão */
    TOKEN_ERROR      = 254   /* Erro de parsing (reservado) */
} TokenType;

typedef struct {
    uint8_t  type;          /* Armazenado em 1 byte para melhor densidade de cache */
    uint16_t value_index;   /* Índice cujo significado depende de `type` (ver acima) */
} Token;

#endif /* TOKENS_H */
