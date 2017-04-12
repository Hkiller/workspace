#ifndef UI_RUNTIME_RENDER_OBJ_REF_I_H
#define UI_RUNTIME_RENDER_OBJ_REF_I_H
#include "render/utils/ui_transform.h"
#include "render/runtime/ui_runtime_render_obj_ref.h"
#include "render/runtime/ui_runtime_render_second_color.h"
#include "ui_runtime_render_obj_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_runtime_render_obj_ref {
    ui_runtime_module_t m_module;
    ui_runtime_render_obj_t m_obj;
    TAILQ_ENTRY(ui_runtime_render_obj_ref) m_next_for_obj;
    ui_runtime_render_obj_evt_fun_t m_evt_fun;
    void * m_evt_ctx;
    uint8_t m_hide;
    struct ui_runtime_render_second_color m_second_color;
    ui_transform m_transform;
};

void ui_runtime_render_obj_ref_disconnect(ui_runtime_render_obj_ref_t obj_ref);

#ifdef __cplusplus
}
#endif

#endif
