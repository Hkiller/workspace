#ifndef UI_RUNTIME_RENDER_QUEUE_H
#define UI_RUNTIME_RENDER_QUEUE_H
#include "ui_runtime_module.h"

#ifdef __cplusplus
extern "C" {
#endif

ui_runtime_render_queue_t ui_runtime_render_queue_create(ui_runtime_render_t render);
void ui_runtime_render_queue_free(ui_runtime_render_queue_t queue);

int ui_runtime_render_queue_push(ui_runtime_render_queue_t queue);
void ui_runtime_render_queue_pop(ui_runtime_render_queue_t queue);

ui_runtime_render_queue_t ui_runtime_render_queue_default(ui_runtime_render_t render);
ui_runtime_render_queue_t ui_runtime_render_queue_top(ui_runtime_render_t render);

void ui_runtime_render_queue_clear(ui_runtime_render_queue_t queue);

#ifdef __cplusplus
}
#endif

#endif
