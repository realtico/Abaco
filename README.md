# Abaco

Uma biblioteca C99 pequena e sem dependências para tokenizar, converter
(Shunting-Yard) e avaliar expressões matemáticas em notação infixa.

Extraída do parser/avaliador do [Multicurvas](https://github.com/realtico/Multicurvas)
para ser reaproveitada em outras aplicações.

## Características

- **Sem dependências** além da libc/libm; C99 puro.
- **Variáveis, funções e constantes definidas pelo chamador** via `AbacoContext`
  — a lib não conhece `x`/`sin`/`pi` de antemão, você registra o que precisar.
- Tabela de funções **builtin** pronta (trigonométricas, hiperbólicas, `log`,
  `sqrt`, `atan2`, `pow`, `min`, `max`, etc.) que você pode usar como está ou
  estender com suas próprias funções.
- Suporte a **funções multi-argumento** (`atan2(y,x)`, `min(a,b)`, ...) com
  validação de aridade no momento da conversão para RPN.
- **Múltiplas variáveis** na mesma expressão (ex.: `f(x,y) = sqrt(x^2+y^2)`).
- Erros diferenciados por categoria (identificador desconhecido, sintaxe,
  aridade, memória na fase de parsing; pilha, divisão por zero, domínio,
  erro matemático na avaliação), com posição do erro no texto original quando
  aplicável.
- Design cache-friendly: tokens de 3 bytes, números num array separado,
  avaliação com pilha estática (sem alocação no caminho quente).
- Locale configurável (`.` ou `,` como marcador decimal).

## Exemplo de uso

```c
#include "context.h"
#include "parser.h"
#include "evaluator.h"

static const char *const variables[] = { "x" };

AbacoContext ctx;
abaco_context_init(&ctx, variables, 1); /* locale POINT + funções/constantes builtin */

TokenBuffer tokens, rpn;
parser_tokenize(&ctx, "sin(x)*2 + sqrt(x)", &tokens, NULL);
parser_to_rpn(&ctx, &tokens, &rpn);

double x = 4.0;
EvalResult result = evaluator_eval_rpn(&ctx, &rpn, &x);
if (result.error == EVAL_OK) {
    printf("%.6g\n", result.value); /* -1.51360 + 2 = ... */
}

parser_free_buffer(&tokens);
parser_free_buffer(&rpn);
```

Para customizar funções/constantes, monte suas próprias tabelas
(`AbacoFunction[]` / `AbacoConstant[]`) e sobrescreva `ctx.functions` /
`ctx.constants` depois de `abaco_context_init`.

## Build

```sh
make            # gera build/libabaco.a
make run-tests  # compila e roda os testes em tests/
```

Para usar em outro projeto: aponte `-I` para `include/` e linke
`build/libabaco.a` (ou compile `src/*.c` diretamente).

## Limitações conhecidas

- Aridade máxima de função: `ABACO_MAX_ARITY` (hoje 2).
- Sem suporte a multiplicação implícita (`2x` não é `2*x`).
- `ParserError`/`error_position` cobrem a fase de tokenização; erros de
  aridade detectados na conversão para RPN não carregam posição no texto.

## Licença

MIT — ver [LICENSE](LICENSE).
