#ifndef UI_RUNTIME_RENDER_OBJ_META_I_H
#define UI_RUNTIME_RENDER_OBJ_META_I_H
#include "render/runtime/ui_runtime_render_obj_meta.h"
#include "ui_runtime_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_runtime_render_obj_meta {
    ui_runtime_module_t m_module;
    struct cpe_hash_entry m_hh;
    const char * m_obj_type_name;
    uint8_t m_obj_type_id;
    size_t m_data_capacity;
    void * m_ctx;
    ui_runtime_render_obj_init_fun_t m_init_fun;
    ui_runtime_render_obj_set_fun_t m_set_fun;
    ui_runtime_render_obj_setup_fun_t m_setup_fun;
    ui_runtime_render_obj_update_fun_t m_update_fun;
    ui_runtime_render_obj_free_fun_t m_free_fun;
    ui_runtime_render_obj_render_fun_t m_render_fun;
    ui_runtime_render_obj_is_playing_fun_t m_is_playing_fun;
    ui_runtime_render_obj_bounding_fun_t m_bounding_fun;
    ui_runtime_render_obj_resize_fun_t m_resize_fun;    
    uint32_t m_obj_count;
    ui_runtime_render_obj_list_t m_objs;
};

uint32_t ui_runtime_render_obj_meta_hash(ui_runtime_render_obj_meta_t obj);
int ui_runtime_render_obj_meta_eq(ui_runtime_render_obj_meta_t l, ui_runtime_render_obj_meta_t r);

void ui_runtime_render_obj_meta_free_all(ui_runtime_module_t module);

#ifdef __cplusplus
}
#endif

#endif
