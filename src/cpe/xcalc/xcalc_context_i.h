#ifndef CPE_XCALC_CONTEXT_I_H
#define CPE_XCALC_CONTEXT_I_H
#include "cpe/xcalc/xcalc_computer.h"
#include "xcalc_token_i.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct xcontext * xcontext_t;

struct xcontext {
    xcomputer_t m_computer;

    xtoken_t m_cur_token;

    xtoken_list_t m_tokens;
    xtoken_t m_sign_token;

    char * m_buf;
    char * m_cur_pos;
};

xcontext_t xcontext_create(xcomputer_t computer, const char * str);
void xcontext_free(xcontext_t context);

int xcontext_get_token(xcontext_t context);

void xcontext_push_token(xcontext_t context);
xtoken_t xcontext_pop_token(xcontext_t context);
xtoken_t xcontext_token_n(xcontext_t context, int n);

void xcontext_update_sign_token(xcontext_t context);

void xcontext_dump_stack(xcontext_t context);

#ifdef __cplusplus
}
#endif

#endif
