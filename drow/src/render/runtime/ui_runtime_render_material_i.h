#ifndef UI_RUNTIME_RENDER_MATERIAL_I_H
#define UI_RUNTIME_RENDER_MATERIAL_I_H
#include "render/runtime/ui_runtime_render_material.h"
#include "ui_runtime_render_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_runtime_render_material {
    ui_runtime_render_t m_render;
    TAILQ_ENTRY(ui_runtime_render_material) m_next;
    ui_runtime_render_state_t m_render_state_parent;
    ui_runtime_render_state_t m_render_state;
    ui_runtime_render_technique_list_t m_techniques;
    ui_runtime_render_technique_t m_cur_technique;
};

void ui_runtime_render_material_real_free(ui_runtime_render_material_t material);
    
#ifdef __cplusplus
}
#endif

#endif
