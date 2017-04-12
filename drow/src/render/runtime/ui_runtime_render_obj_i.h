#ifndef UI_RUNTIME_RENDER_OBJ_I_H
#define UI_RUNTIME_RENDER_OBJ_I_H
#include "render/utils/ui_transform.h"
#include "render/utils/ui_vector_2.h"
#include "render/runtime/ui_runtime_render_obj.h"
#include "ui_runtime_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_runtime_render_obj {
    ui_runtime_module_t m_module;
    struct cpe_hash_entry m_hh_for_module;
    ui_data_src_t m_src;
    TAILQ_ENTRY(ui_runtime_render_obj) m_next_for_module;
    TAILQ_ENTRY(ui_runtime_render_obj) m_next_for_update;
    ui_runtime_render_obj_meta_t m_meta;
    TAILQ_ENTRY(ui_runtime_render_obj) m_next_for_meta;
    ui_runtime_render_obj_ref_list_t m_refs;
    ui_runtime_render_obj_child_list_t m_childs;
    ui_runtime_render_obj_evt_fun_t m_evt_fun;
    void * m_evt_ctx;

    /*touch*/
    ui_runtime_touch_process_fun_t m_touch_process_fun;
    void * m_touch_process_ctx;

    /*runtime*/
    ui_vector_2 m_size;
    char m_name[64];
    float m_time_scale;
    uint16_t m_ref_count;
    uint8_t m_auto_release;
    uint8_t m_suspend;
    uint8_t m_keep_update;
    uint8_t m_in_module_update;
    ui_transform m_transform;
    ui_runtime_render_obj_ref_t m_updator;
};

uint32_t ui_runtime_render_obj_name_hash(ui_runtime_render_obj_t obj);
int ui_runtime_render_obj_name_eq(ui_runtime_render_obj_t l, ui_runtime_render_obj_t r);

void ui_runtime_render_obj_sync_module_update(ui_runtime_render_obj_t obj);

#ifdef __cplusplus
}
#endif

#endif
