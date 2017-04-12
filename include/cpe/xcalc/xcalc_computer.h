#ifndef CPE_XCALC_COMPUTER_H
#define CPE_XCALC_COMPUTER_H
#include "cpe/utils/error.h"
#include "cpe/utils/memory.h"
#include "xcalc_types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct xcomputer_args {
    void * m_ctx;
    xtoken_t (*m_find_arg)(void * ctx, xcomputer_t computer, const char * arg_name, error_monitor_t em);
};

typedef xtoken_t (*xcalc_func_t)(void * ctx, xcomputer_t computer, const char * func_name, xtoken_it_t args, error_monitor_t em);

xcomputer_t xcomputer_create(mem_allocrator_t alloc, error_monitor_t em);
void xcomputer_free(xcomputer_t computer);

xtoken_t xcomputer_alloc_token(xcomputer_t computer);
void xcomputer_free_token(xcomputer_t computer , xtoken_t token);

int xcomputer_add_func(xcomputer_t computer, const char * func_name, xcalc_func_t fun, void * fun_ctx);
void xcomputer_remove_func_by_name(xcomputer_t computer, const char * func_name);
void xcomputer_remove_func_by_ctx(xcomputer_t computer, void * ctx);

xtoken_t xcomputer_compute(xcomputer_t computer, const char * str, xcomputer_args_t args);

int xcomputer_compute_bool(xcomputer_t computer, const char * str, xcomputer_args_t args, uint8_t * r);

void xcomputer_set_token_float(xcomputer_t computer, xtoken_t token, double v);
void xcomputer_set_token_int(xcomputer_t computer, xtoken_t token, int64_t v);
int xcomputer_set_token_str(xcomputer_t computer, xtoken_t token, const char * v);
int xcomputer_set_token_str_range(xcomputer_t computer, xtoken_t token, const char * begin, const char * end);

int xcomputer_visit_args(xcomputer_t computer, const char * str, void * ctx, void (*on_found)(void * ctx, xtoken_t arg));

struct xcomputer_find_value_from_str_ctx {
    const char * m_str;
    uint8_t m_sep;
    uint8_t m_pair;
};

xtoken_t xcomputer_find_value_from_str(void * input_ctx, xcomputer_t computer, const char * attr_name, error_monitor_t em);

#ifdef __cplusplus
}
#endif

#endif
