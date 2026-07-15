#ifndef ABACO_TEST_H
#define ABACO_TEST_H

/* Mini test-runner sem dependências externas: contabiliza pass/fail e
 * devolve um exit code != 0 se algo falhar, em vez de abortar no
 * primeiro assert() (útil para rodar em CI e ver todas as falhas de uma vez). */

typedef struct {
    int total;
    int failed;
} AbacoTestStats;

extern AbacoTestStats abaco_test_stats;

void abaco_test_check(int passed, const char *expr, const char *file, int line);
void abaco_test_check_near(double got, double expected, double eps,
                            const char *expr_got, const char *expr_expected,
                            const char *file, int line);

/* Imprime o resumo da suíte e retorna 0 (tudo passou) ou 1 (houve falha) —
 * use como valor de retorno de main(). */
int abaco_test_summary(const char *suite_name);

#define ABACO_ASSERT(cond) \
    abaco_test_check((cond), #cond, __FILE__, __LINE__)

#define ABACO_ASSERT_NEAR(got, expected, eps) \
    abaco_test_check_near((got), (expected), (eps), #got, #expected, __FILE__, __LINE__)

#endif /* ABACO_TEST_H */
