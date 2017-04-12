#ifndef UI_RUNTIME_RENDER_WORKER_H
#define UI_RUNTIME_RENDER_WORKER_H
#include "ui_runtime_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*ui_runtime_render_worker_begin_fun_t)(void * ctx);
typedef void (*ui_runtime_render_worker_end_fun_t)(void * ctx);
    
ui_runtime_render_worker_t
ui_runtime_render_worker_create(
    ui_runtime_render_t context,
    void * ctx,
    ui_runtime_render_worker_begin_fun_t begin_fun,
    ui_runtime_render_worker_end_fun_t end_fun,
    uint8_t with_thread);

void ui_runtime_render_worker_free(ui_runtime_render_worker_t worker);

ui_runtime_render_worker_t
ui_runtime_render_worker_get(ui_runtime_render_t context);
    
void ui_runtime_render_work_run(ui_runtime_render_worker_t worker, uint32_t loop_count, uint32_t timespan_ms);
    
#ifdef __cplusplus
}
#endif

#endif
