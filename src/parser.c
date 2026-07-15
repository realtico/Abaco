#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "parser.h"

/* Forward declarations */
static ParserError parser_check_syntax(const TokenBuffer *tokens, const int *positions, int *error_position);
static int is_operator(TokenType type);
static int get_precedence(TokenType type);

/* Inicializa buffer */
void parser_init_buffer(TokenBuffer *buf) {
    buf->capacity = 64;
    buf->size = 0;
    buf->tokens = malloc(buf->capacity * sizeof(Token));

    buf->values_capacity = 16;  /* Menos valores que tokens */
    buf->values_size = 0;
    buf->values = malloc(buf->values_capacity * sizeof(double));
}

/* Libera buffer */
void parser_free_buffer(TokenBuffer *buf) {
    free(buf->tokens);
    buf->tokens = NULL;
    buf->size = 0;
    buf->capacity = 0;

    free(buf->values);
    buf->values = NULL;
    buf->values_size = 0;
    buf->values_capacity = 0;
}

/* Adiciona token ao buffer (com realocação se necessário) */
int parser_add_token(TokenBuffer *buf, Token token) {
    if (buf->size >= buf->capacity) {
        buf->capacity *= 2;
        Token *new_tokens = realloc(buf->tokens, buf->capacity * sizeof(Token));
        if (!new_tokens) return 0;
        buf->tokens = new_tokens;
    }
    buf->tokens[buf->size++] = token;
    return 1;
}

/* Adiciona valor numérico ao buffer e retorna o índice */
static int parser_add_value(TokenBuffer *buf, double value) {
    if (buf->values_size >= buf->values_capacity) {
        buf->values_capacity *= 2;
        double *new_values = realloc(buf->values, buf->values_capacity * sizeof(double));
        if (!new_values) return -1;
        buf->values = new_values;
    }
    buf->values[buf->values_size] = value;
    return buf->values_size++;
}

/* Registra a posição de origem (no texto) do token que acabou de ser adicionado.
 * Usado apenas durante parser_tokenize, para mensagens de erro com localização. */
static int record_position(int **positions, int *capacity, int index, int pos_value) {
    if (index >= *capacity) {
        int new_capacity = (*capacity) * 2;
        int *tmp = realloc(*positions, new_capacity * sizeof(int));
        if (!tmp) return 0;
        *positions = tmp;
        *capacity = new_capacity;
    }
    (*positions)[index] = pos_value;
    return 1;
}

/* Tenta fazer parse de um número */
static int try_parse_number(const AbacoContext *ctx, const char *str, int *pos, double *value) {
    char buffer[64];  /* Buffer para normalizar marca decimal */
    const char *src = str + *pos;
    char *dst = buffer;
    char dec_mark = (ctx->locale == LOCALE_COMMA) ? ',' : '.';
    int has_decimal = 0;
    int digit_count = 0;

    /* Copia dígitos normalizando marca decimal para ponto (padrão C) */
    while (dst - buffer < (int)sizeof(buffer) - 2 &&
           (isdigit((unsigned char)*src) || (*src == dec_mark && !has_decimal))) {

        if (*src == dec_mark) {
            *dst++ = '.';  /* Normaliza para ponto */
            has_decimal = 1;
            src++;
        } else {
            *dst++ = *src++;
            digit_count++;
        }
    }
    *dst = '\0';

    /* Precisa ter pelo menos um dígito */
    if (digit_count == 0) {
        return 0;
    }

    char *endptr;
    double num = strtod(buffer, &endptr);

    if (endptr == buffer) {
        return 0;
    }

    *value = num;
    *pos += (endptr - buffer);

    return 1;
}

/* Casa `name` em str+pos, respeitando limite de palavra (próximo char não pode ser alnum) */
static int match_word(const char *str, int pos, const char *name) {
    size_t len = strlen(name);
    if (strncmp(str + pos, name, len) != 0) return 0;
    if (isalnum((unsigned char)str[pos + len])) return 0;
    return 1;
}

/* Tokenizador principal */
ParserError parser_tokenize(const AbacoContext *ctx, const char *expr, TokenBuffer *output, int *error_position) {
    if (error_position) *error_position = -1;
    if (!ctx || !expr || !output) return PARSER_SYNTAX_ERROR;

    parser_init_buffer(output);
    if (!output->tokens || !output->values) {
        parser_free_buffer(output);
        return PARSER_MEMORY_ERROR;
    }

    int positions_capacity = 64;
    int *positions = malloc(positions_capacity * sizeof(int));
    if (!positions) {
        parser_free_buffer(output);
        return PARSER_MEMORY_ERROR;
    }

/* Adiciona `tok` à saída e registra sua posição de origem `start`.
 * Em caso de falha de memória, limpa tudo e retorna PARSER_MEMORY_ERROR. */
#define EMIT(tok, start) \
    do { \
        if (!parser_add_token(output, (tok)) || \
            !record_position(&positions, &positions_capacity, output->size - 1, (start))) { \
            free(positions); \
            parser_free_buffer(output); \
            return PARSER_MEMORY_ERROR; \
        } \
    } while (0)

    char dec_mark = (ctx->locale == LOCALE_COMMA) ? ',' : '.';
    int i = 0;
    while (expr[i] != '\0') {
        /* Pula espaços em branco */
        if (isspace((unsigned char)expr[i])) {
            i++;
            continue;
        }

        Token token = {0, 0};
        int start = i;

        /* Tenta fazer parse de número */
        if (isdigit((unsigned char)expr[i]) || (expr[i] == dec_mark && isdigit((unsigned char)expr[i+1]))) {
            double value;
            if (try_parse_number(ctx, expr, &i, &value)) {
                int value_idx = parser_add_value(output, value);
                if (value_idx < 0) {
                    free(positions);
                    parser_free_buffer(output);
                    return PARSER_MEMORY_ERROR;
                }
                token.type = TOKEN_NUMBER;
                token.value_index = (uint16_t)value_idx;
                EMIT(token, start);
                continue;
            }
        }

        /* Tenta fazer parse de identificador (constante, variável ou função) */
        if (isalpha((unsigned char)expr[i])) {
            int matched = 0;

            for (int c = 0; c < ctx->constant_count && !matched; c++) {
                if (match_word(expr, i, ctx->constants[c].name)) {
                    int value_idx = parser_add_value(output, ctx->constants[c].value);
                    if (value_idx < 0) {
                        free(positions);
                        parser_free_buffer(output);
                        return PARSER_MEMORY_ERROR;
                    }
                    token.type = TOKEN_NUMBER;
                    token.value_index = (uint16_t)value_idx;
                    i += (int)strlen(ctx->constants[c].name);
                    matched = 1;
                }
            }

            for (int v = 0; v < ctx->variable_count && !matched; v++) {
                if (match_word(expr, i, ctx->variables[v])) {
                    token.type = TOKEN_VARIABLE;
                    token.value_index = (uint16_t)v;
                    i += (int)strlen(ctx->variables[v]);
                    matched = 1;
                }
            }

            for (int f = 0; f < ctx->function_count && !matched; f++) {
                if (match_word(expr, i, ctx->functions[f].name)) {
                    token.type = TOKEN_FUNCTION;
                    token.value_index = (uint16_t)f;
                    i += (int)strlen(ctx->functions[f].name);
                    matched = 1;
                }
            }

            if (!matched) {
                free(positions);
                parser_free_buffer(output);
                if (error_position) *error_position = start;
                return PARSER_UNKNOWN_IDENTIFIER;
            }

            EMIT(token, start);
            continue;
        }

        /* Caracteres especiais e operadores */
        switch (expr[i]) {
            /* Tratamento especial para '+' porque pode ser unário */
            case '+': {
                /* Detecta se é operador unário (positivo) */
                int is_unary_plus = 0;
                if (output->size == 0) {
                    is_unary_plus = 1;
                } else {
                    TokenType prev = output->tokens[output->size - 1].type;
                    if (prev == TOKEN_LPAREN || prev == TOKEN_COMMA || prev == TOKEN_PLUS ||
                        prev == TOKEN_MINUS || prev == TOKEN_MULT ||
                        prev == TOKEN_DIV || prev == TOKEN_POW ||
                        prev == TOKEN_NEG) {
                        is_unary_plus = 1;
                    }
                }

                if (is_unary_plus) {
                    /* Unary plus is a no-op: skip the '+' token entirely */
                    i++;
                    break;
                }

                /* Caso não-unário: adiciona o operador plus normalmente */
                token.type = TOKEN_PLUS;
                EMIT(token, start);
                i++;
                break;
            }
            case '*': case '/': case '^':
            case '(': case ')': case ',':
                token.type = (TokenType)expr[i];
                EMIT(token, start);
                i++;
                break;

            case '-': {
                /* Detecta se é operador unário (negativo) */
                int is_unary = 0;
                if (output->size == 0) {
                    is_unary = 1;
                } else {
                    TokenType prev = output->tokens[output->size - 1].type;
                    if (prev == TOKEN_LPAREN || prev == TOKEN_COMMA || prev == TOKEN_PLUS ||
                        prev == TOKEN_MINUS || prev == TOKEN_MULT ||
                        prev == TOKEN_DIV || prev == TOKEN_POW ||
                        prev == TOKEN_NEG) {
                        is_unary = 1;
                    }
                }

                if (is_unary) {
                    /* Emite token de negação unária (prefix) */
                    token.type = TOKEN_NEG;
                    EMIT(token, start);
                    i++;
                    break;
                }

                /* Caso binário: adiciona o operador menos */
                token.type = TOKEN_MINUS;
                EMIT(token, start);
                i++;
                break;
            }

            default:
                /* Caractere inválido */
                free(positions);
                parser_free_buffer(output);
                if (error_position) *error_position = start;
                return PARSER_SYNTAX_ERROR;
        }
    }

    /* Adiciona token de fim */
    Token end_token = { TOKEN_END, 0 };
    EMIT(end_token, i);

#undef EMIT

    /* Valida sintaxe (parênteses balanceados) */
    ParserError err = parser_check_syntax(output, positions, error_position);
    free(positions);
    if (err != PARSER_OK) {
        parser_free_buffer(output);
        return err;
    }

    return PARSER_OK;
}

/* Verifica parênteses balanceados; aponta a posição do primeiro parêntese
 * sem par correspondente (seja um ')' sobrando ou um '(' nunca fechado). */
static ParserError parser_check_syntax(const TokenBuffer *tokens, const int *positions, int *error_position) {
    int *open_stack = malloc((tokens->size > 0 ? tokens->size : 1) * sizeof(int));
    if (!open_stack) return PARSER_MEMORY_ERROR;
    int top = -1;
    ParserError result = PARSER_OK;

    for (int i = 0; i < tokens->size - 1; i++) {  /* -1 para pular TOKEN_END */
        TokenType curr = tokens->tokens[i].type;

        if (curr == TOKEN_LPAREN) {
            open_stack[++top] = i;
        } else if (curr == TOKEN_RPAREN) {
            if (top < 0) {
                if (error_position) *error_position = positions[i];
                result = PARSER_SYNTAX_ERROR;
                goto done;
            }
            top--;
        }
    }

    if (top >= 0) {
        if (error_position) *error_position = positions[open_stack[0]];
        result = PARSER_SYNTAX_ERROR;
    }

done:
    free(open_stack);
    return result;
}

/* Retorna precedência de um operador (maior = mais prioritário) */
static int get_precedence(TokenType type) {
    switch (type) {
        case TOKEN_POW:
            return 4;
        case TOKEN_NEG:
            return 5; /* Negação unária tem precedência maior */
        case TOKEN_MULT:
        case TOKEN_DIV:
            return 3;
        case TOKEN_PLUS:
        case TOKEN_MINUS:
            return 2;
        default:
            return 0;
    }
}

/* Verifica se o token é um operador */
static int is_operator(TokenType type) {
    return (type == TOKEN_PLUS || type == TOKEN_MINUS ||
            type == TOKEN_MULT || type == TOKEN_DIV || type == TOKEN_POW ||
            type == TOKEN_NEG);
}

/* Algoritmo Shunting Yard - Converte infixa para RPN */
ParserError parser_to_rpn(const AbacoContext *ctx, TokenBuffer *tokens, TokenBuffer *rpn) {
    if (!ctx || !tokens || !rpn) return PARSER_SYNTAX_ERROR;

    parser_init_buffer(rpn);

    /* Copia o array de valores (números e constantes já resolvidas) */
    if (tokens->values_size > 0) {
        rpn->values = malloc(tokens->values_size * sizeof(double));
        if (!rpn->values) {
            parser_free_buffer(rpn);
            return PARSER_MEMORY_ERROR;
        }
        memcpy(rpn->values, tokens->values, tokens->values_size * sizeof(double));
        rpn->values_size = tokens->values_size;
        rpn->values_capacity = tokens->values_size;
    }

    /* Pilha de operadores/funções/parênteses */
    Token *stack = malloc((tokens->size > 0 ? tokens->size : 1) * sizeof(Token));
    /* Paralelos à pilha, indexados pela posição do '(' correspondente:
     * is_call = 1 se este '(' abre uma chamada de função (não apenas agrupamento);
     * arg_count = número de argumentos vistos até agora (via vírgulas) */
    int *is_call = malloc((tokens->size > 0 ? tokens->size : 1) * sizeof(int));
    int *arg_count = malloc((tokens->size > 0 ? tokens->size : 1) * sizeof(int));
    if (!stack || !is_call || !arg_count) {
        free(stack); free(is_call); free(arg_count);
        parser_free_buffer(rpn);
        return PARSER_MEMORY_ERROR;
    }
    int stack_top = -1;
    ParserError result = PARSER_OK;

    /* Processa cada token */
    for (int i = 0; i < tokens->size; i++) {
        Token token = tokens->tokens[i];
        TokenType type = token.type;

        /* Fim da expressão - para de processar */
        if (type == TOKEN_END) break;

        /* Números e variáveis vão direto para a saída */
        if (type == TOKEN_NUMBER || type == TOKEN_VARIABLE) {
            if (!parser_add_token(rpn, token)) { result = PARSER_MEMORY_ERROR; goto cleanup; }
        }
        /* Funções vão para a pilha */
        else if (type == TOKEN_FUNCTION) {
            stack[++stack_top] = token;
        }
        /* Parêntese esquerdo vai para a pilha */
        else if (type == TOKEN_LPAREN) {
            int call = (stack_top >= 0 && stack[stack_top].type == TOKEN_FUNCTION);
            stack[++stack_top] = token;
            is_call[stack_top] = call;
            arg_count[stack_top] = call ? 1 : 0;
        }
        /* Vírgula: fecha o argumento atual (desempilha até o '(' da chamada) */
        else if (type == TOKEN_COMMA) {
            while (stack_top >= 0 && stack[stack_top].type != TOKEN_LPAREN) {
                if (!parser_add_token(rpn, stack[stack_top--])) { result = PARSER_MEMORY_ERROR; goto cleanup; }
            }
            if (stack_top < 0) { result = PARSER_SYNTAX_ERROR; goto cleanup; }
            arg_count[stack_top]++;
        }
        /* Parêntese direito: desempilha até encontrar '(' */
        else if (type == TOKEN_RPAREN) {
            while (stack_top >= 0 && stack[stack_top].type != TOKEN_LPAREN) {
                if (!parser_add_token(rpn, stack[stack_top--])) { result = PARSER_MEMORY_ERROR; goto cleanup; }
            }
            if (stack_top < 0) { result = PARSER_SYNTAX_ERROR; goto cleanup; }

            int call = is_call[stack_top];
            int args = arg_count[stack_top];
            stack_top--; /* remove o '(' */

            if (call) {
                if (stack_top < 0 || stack[stack_top].type != TOKEN_FUNCTION) {
                    result = PARSER_SYNTAX_ERROR; goto cleanup;
                }
                Token fn_token = stack[stack_top--];
                const AbacoFunction *fn = &ctx->functions[fn_token.value_index];
                if (args != fn->arity) { result = PARSER_ARITY_ERROR; goto cleanup; }
                if (!parser_add_token(rpn, fn_token)) { result = PARSER_MEMORY_ERROR; goto cleanup; }
            }
        }
        /* Operadores */
        else if (is_operator(type)) {
            /* Desempilha operadores de maior ou igual precedência */
            /* Exceto para ^ e NEG, que são associativos à direita */
            int prec = get_precedence(type);

            while (stack_top >= 0 && is_operator(stack[stack_top].type)) {
                int stack_prec = get_precedence(stack[stack_top].type);

                if (type == TOKEN_POW || type == TOKEN_NEG) {
                    if (stack_prec <= prec) break;
                } else {
                    if (stack_prec < prec) break;
                }

                if (!parser_add_token(rpn, stack[stack_top--])) { result = PARSER_MEMORY_ERROR; goto cleanup; }
            }

            stack[++stack_top] = token;
        }
    }

    /* Desempilha todos os operadores/funções restantes */
    while (stack_top >= 0) {
        if (stack[stack_top].type == TOKEN_LPAREN) { result = PARSER_SYNTAX_ERROR; goto cleanup; }
        if (!parser_add_token(rpn, stack[stack_top--])) { result = PARSER_MEMORY_ERROR; goto cleanup; }
    }

    /* Adiciona token de fim */
    {
        Token end_token = { TOKEN_END, 0 };
        if (!parser_add_token(rpn, end_token)) { result = PARSER_MEMORY_ERROR; goto cleanup; }
    }

cleanup:
    free(stack);
    free(is_call);
    free(arg_count);
    if (result != PARSER_OK) {
        parser_free_buffer(rpn);
        return result;
    }
    return PARSER_OK;
}
