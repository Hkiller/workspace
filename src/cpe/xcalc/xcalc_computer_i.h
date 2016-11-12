#ifndef CPE_XCALC_COMPUTER_I_H
#define CPE_XCALC_COMPUTER_I_H
#include "cpe/utils/hash.h"
#include "cpe/utils/buffer.h"
#include "cpe/xcalc/xcalc_computer.h"
#include "xcalc_token_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct xcomputer {
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    struct cpe_hash_table m_funcs;

    uint32_t m_allocked_token_count;

    struct mem_buffer m_tmp_buffer;
};
int xcomputer_dup_token_str(xcomputer_t computer , xtoken_t token);

/*xcomputer_fun_def*/
struct xcomputer_fun_def {
    const char * m_func_name;
    xcalc_func_t m_fun;
    void * m_fun_ctx;
    struct cpe_hash_entry m_hh;
};
uint32_t xcomputer_fun_def_hash(struct xcomputer_fun_def * func_def);
int xcomputer_fun_def_eq(struct xcomputer_fun_def * l, struct xcomputer_fun_def * r);
void xcomputer_func_def_free_all(xcomputer_t computer);
struct xcomputer_fun_def * xcomputer_find_fun_def(xcomputer_t computer, const char * name);

/*xtoken_it*/
struct xtoken_it {
    xtoken_list_t m_not_visited;
    xtoken_list_t m_visited;
};

void xcomputer_token_it_init(xtoken_it_t token_it);
void xcomputer_token_it_fini(xcomputer_t computer, xtoken_it_t token_it);

/*internal funcs */
int xcomputer_load_default_funcs(xcomputer_t computer);

#ifdef __cplusplus
}
#endif

#endif
