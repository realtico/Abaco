#include <stdio.h>
#include <math.h>
#include "abaco_test.h"

AbacoTestStats abaco_test_stats = { 0, 0 };

void abaco_test_check(int passed, const char *expr, const char *file, int line) {
    abaco_test_stats.total++;
    if (!passed) {
        abaco_test_stats.failed++;
        fprintf(stderr, "  FALHOU  %s:%d: %s\n", file, line, expr);
    }
}

void abaco_test_check_near(double got, double expected, double eps,
                            const char *expr_got, const char *expr_expected,
                            const char *file, int line) {
    abaco_test_stats.total++;
    if (fabs(got - expected) > eps) {
        abaco_test_stats.failed++;
        fprintf(stderr, "  FALHOU  %s:%d: %s (%.10g) != %s (%.10g) [eps=%.3g]\n",
                file, line, expr_got, got, expr_expected, expected, eps);
    }
}

int abaco_test_summary(const char *suite_name) {
    int passed = abaco_test_stats.total - abaco_test_stats.failed;
    printf("\n%s: %d/%d testes passaram\n", suite_name, passed, abaco_test_stats.total);
    return abaco_test_stats.failed == 0 ? 0 : 1;
}
