#ifndef UI_RUNTIME_RENDER_STATE_I_H
#define UI_RUNTIME_RENDER_STATE_I_H
#include "render/runtime/ui_runtime_render_state.h"
#include "ui_runtime_render_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_runtime_render_state {
    ui_runtime_render_t m_render;
    union {
        struct {
            ui_runtime_render_state_t m_parent;
            struct ui_runtime_render_state_data m_data;
        };
        TAILQ_ENTRY(ui_runtime_render_state) m_next;
    };
};

ui_runtime_render_state_t ui_runtime_render_state_create(ui_runtime_render_t render, ui_runtime_render_state_t parent);
ui_runtime_render_state_t ui_runtime_render_state_clone(ui_runtime_render_state_t proto, ui_runtime_render_state_t parent);
void ui_runtime_render_state_free(ui_runtime_render_state_t state);

void ui_runtime_render_state_real_free(ui_runtime_render_state_t state);

#ifdef __cplusplus
}
#endif

#endif
