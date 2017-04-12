#ifndef UI_RUNTIME_RENDER_TECHNIQUE_I_H
#define UI_RUNTIME_RENDER_TECHNIQUE_I_H
#include "render/runtime/ui_runtime_render_technique.h"
#include "ui_runtime_render_material_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_runtime_render_technique {
    ui_runtime_render_material_t m_material;
    TAILQ_ENTRY(ui_runtime_render_technique) m_next;
    uint8_t m_pass_count;
    ui_runtime_render_state_t m_render_state;
    ui_runtime_render_pass_list_t m_passes;
};

void ui_runtime_render_technique_real_free(ui_runtime_render_technique_t technique);
    
#ifdef __cplusplus
}
#endif

#endif
