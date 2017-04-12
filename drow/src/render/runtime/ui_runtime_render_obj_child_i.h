#ifndef UI_RUNTIME_RENDER_OBJ_CHILD_I_H
#define UI_RUNTIME_RENDER_OBJ_CHILD_I_H
#include "ui_runtime_render_obj_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_runtime_render_obj_child {
    ui_runtime_render_obj_t m_obj;
    TAILQ_ENTRY(ui_runtime_render_obj_child) m_next_for_obj;
    cpe_hash_entry m_hh_for_module;
    ui_runtime_render_obj_ref_t m_child_obj_ref;
    void const * m_tag;
    uint8_t m_auto_render;
};

ui_runtime_render_obj_child_t
ui_runtime_render_obj_child_create(ui_runtime_render_obj_t obj, ui_runtime_render_obj_ref_t child_obj_ref, void const * tag, uint8_t auto_render);
void ui_runtime_render_obj_child_free(ui_runtime_render_obj_child_t child);

void ui_runtime_render_obj_child_real_free(ui_runtime_render_obj_child_t child);

uint32_t ui_runtime_render_obj_child_hash(ui_runtime_render_obj_child_t obj);
int ui_runtime_render_obj_child_eq(ui_runtime_render_obj_child_t l, ui_runtime_render_obj_child_t r);

#ifdef __cplusplus
}
#endif

#endif
