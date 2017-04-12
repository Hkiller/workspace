#include <assert.h>
#include "gd/app/app_context.h"
#include "app_internal_ops.h"

int gd_app_tick_add(gd_app_context_t context, gd_app_tick_fun tick, void * tick_ctx, ptr_int_t tick_arg) {
    struct gd_app_ticker * ticker;

    ticker = (struct gd_app_ticker *)mem_alloc(context->m_alloc, sizeof(struct gd_app_ticker));
    if (ticker == NULL) return -1;

    ticker->m_tick = tick;
    ticker->m_ctx = tick_ctx;
    ticker->m_arg = tick_arg;

    TAILQ_INSERT_TAIL(&context->m_tick_chain, ticker, m_next);

    return 0;
}

int gd_app_tick_remove(gd_app_context_t context, gd_app_tick_fun tick, void * tick_ctx) {
    struct gd_app_ticker * ticker;

    TAILQ_FOREACH(ticker, &context->m_tick_chain, m_next) {
        if (ticker->m_tick == tick && ticker->m_ctx == tick_ctx) {
            TAILQ_REMOVE(&context->m_tick_chain, ticker, m_next);
            mem_free(context->m_alloc, ticker);
            return 0;
        }
    }

    return -1;
}

void gd_app_tick_chain_free(gd_app_context_t context) {
    while(!TAILQ_EMPTY(&context->m_tick_chain)) {
        struct gd_app_ticker * ticker = TAILQ_FIRST(&context->m_tick_chain);
        gd_app_tick_remove(context, ticker->m_tick, ticker->m_ctx);
    }
}

