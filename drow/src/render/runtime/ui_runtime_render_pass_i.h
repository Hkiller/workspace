#ifndef UI_RUNTIME_RENDER_PASS_I_H
#define UI_RUNTIME_RENDER_PASS_I_H
#include "render/runtime/ui_runtime_render_pass.h"
#include "ui_runtime_render_technique_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_runtime_render_pass {
    ui_runtime_render_technique_t m_technique;
    TAILQ_ENTRY(ui_runtime_render_pass) m_next;
    ui_runtime_render_state_t m_render_state;
    ui_runtime_render_program_state_t m_program_state;
};

void ui_runtime_render_pass_real_free(ui_runtime_render_pass_t pass);
    
#ifdef __cplusplus
}
#endif

#endif
