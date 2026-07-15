#include <math.h>
#include "context.h"

#define M_PI_CUSTOM 3.14159265358979323846
#define M_E_CUSTOM  2.71828182845904523536

static AbacoFnResult fn_ok(double value) {
    AbacoFnResult r = { ABACO_FN_OK, value };
    return r;
}

static AbacoFnResult fn_domain_error(void) {
    AbacoFnResult r = { ABACO_FN_DOMAIN_ERROR, 0.0 };
    return r;
}

/* Funções unárias */
static AbacoFnResult fn_sin(const double *a)   { return fn_ok(sin(a[0])); }
static AbacoFnResult fn_cos(const double *a)   { return fn_ok(cos(a[0])); }
static AbacoFnResult fn_tan(const double *a)   { return fn_ok(tan(a[0])); }
static AbacoFnResult fn_abs(const double *a)   { return fn_ok(fabs(a[0])); }
static AbacoFnResult fn_sqrt(const double *a)  { return a[0] < 0.0 ? fn_domain_error() : fn_ok(sqrt(a[0])); }
static AbacoFnResult fn_exp(const double *a)   { return fn_ok(exp(a[0])); }
static AbacoFnResult fn_log(const double *a)   { return a[0] <= 0.0 ? fn_domain_error() : fn_ok(log(a[0])); }
static AbacoFnResult fn_log10(const double *a) { return a[0] <= 0.0 ? fn_domain_error() : fn_ok(log10(a[0])); }
static AbacoFnResult fn_sinh(const double *a)  { return fn_ok(sinh(a[0])); }
static AbacoFnResult fn_cosh(const double *a)  { return fn_ok(cosh(a[0])); }
static AbacoFnResult fn_tanh(const double *a)  { return fn_ok(tanh(a[0])); }
static AbacoFnResult fn_asin(const double *a)  { return (a[0] < -1.0 || a[0] > 1.0) ? fn_domain_error() : fn_ok(asin(a[0])); }
static AbacoFnResult fn_acos(const double *a)  { return (a[0] < -1.0 || a[0] > 1.0) ? fn_domain_error() : fn_ok(acos(a[0])); }
static AbacoFnResult fn_atan(const double *a)  { return fn_ok(atan(a[0])); }
static AbacoFnResult fn_asinh(const double *a) { return fn_ok(asinh(a[0])); }
static AbacoFnResult fn_acosh(const double *a) { return a[0] < 1.0 ? fn_domain_error() : fn_ok(acosh(a[0])); }
static AbacoFnResult fn_atanh(const double *a) { return (a[0] <= -1.0 || a[0] >= 1.0) ? fn_domain_error() : fn_ok(atanh(a[0])); }
static AbacoFnResult fn_ceil(const double *a)  { return fn_ok(ceil(a[0])); }
static AbacoFnResult fn_floor(const double *a) { return fn_ok(floor(a[0])); }
static AbacoFnResult fn_frac(const double *a)  { return fn_ok(a[0] - floor(a[0])); }

/* Funções binárias */
static AbacoFnResult fn_atan2(const double *a) { return fn_ok(atan2(a[0], a[1])); }
static AbacoFnResult fn_pow(const double *a)   { double v = pow(a[0], a[1]); return isnan(v) ? fn_domain_error() : fn_ok(v); }
static AbacoFnResult fn_min(const double *a)   { return fn_ok(a[0] < a[1] ? a[0] : a[1]); }
static AbacoFnResult fn_max(const double *a)   { return fn_ok(a[0] > a[1] ? a[0] : a[1]); }

const AbacoFunction ABACO_BUILTIN_FUNCTIONS[] = {
    { "sin",   1, fn_sin },
    { "cos",   1, fn_cos },
    { "tan",   1, fn_tan },
    { "abs",   1, fn_abs },
    { "sqrt",  1, fn_sqrt },
    { "exp",   1, fn_exp },
    { "log10", 1, fn_log10 },
    { "log",   1, fn_log },
    { "ln",    1, fn_log },   /* alias de log (logaritmo natural) */
    { "sinh",  1, fn_sinh },
    { "cosh",  1, fn_cosh },
    { "tanh",  1, fn_tanh },
    { "asinh", 1, fn_asinh },
    { "acosh", 1, fn_acosh },
    { "atanh", 1, fn_atanh },
    { "asin",  1, fn_asin },
    { "acos",  1, fn_acos },
    { "atan",  1, fn_atan },
    { "ceil",  1, fn_ceil },
    { "floor", 1, fn_floor },
    { "frac",  1, fn_frac },
    { "atan2", 2, fn_atan2 },
    { "pow",   2, fn_pow },
    { "min",   2, fn_min },
    { "max",   2, fn_max },
};
const int ABACO_BUILTIN_FUNCTION_COUNT = sizeof(ABACO_BUILTIN_FUNCTIONS) / sizeof(ABACO_BUILTIN_FUNCTIONS[0]);

const AbacoConstant ABACO_BUILTIN_CONSTANTS[] = {
    { "pi", M_PI_CUSTOM },
    { "e",  M_E_CUSTOM },
};
const int ABACO_BUILTIN_CONSTANT_COUNT = sizeof(ABACO_BUILTIN_CONSTANTS) / sizeof(ABACO_BUILTIN_CONSTANTS[0]);

void abaco_context_init(AbacoContext *ctx, const char *const *variables, int variable_count) {
    ctx->locale = LOCALE_POINT;
    ctx->variables = variables;
    ctx->variable_count = variable_count;
    ctx->functions = ABACO_BUILTIN_FUNCTIONS;
    ctx->function_count = ABACO_BUILTIN_FUNCTION_COUNT;
    ctx->constants = ABACO_BUILTIN_CONSTANTS;
    ctx->constant_count = ABACO_BUILTIN_CONSTANT_COUNT;
}
