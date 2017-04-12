#ifndef UI_RUNTIME_CONTEXT_WORKER_I_H
#define UI_RUNTIME_CONTEXT_WORKER_I_H
#include <pthread.h>
#include "render/runtime/ui_runtime_render_worker.h"
#include "ui_runtime_render_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_runtime_render_worker {
    ui_runtime_render_t m_context;
    pthread_mutex_t m_mutex;
    pthread_cond_t m_cond;
    pthread_t m_thread;
    uint8_t m_with_thread;
    uint8_t m_is_exit;
    uint8_t m_is_rending;
    void * m_ctx;
    ui_runtime_render_worker_begin_fun_t m_begin_fun;
    ui_runtime_render_worker_end_fun_t m_end_fun;
};

void ui_runtime_render_worker_signal(ui_runtime_render_worker_t worker);

#ifdef __cplusplus
}
#endif

#endif
