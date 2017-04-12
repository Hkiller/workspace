#ifndef UI_MODEL_ED_OBJ_META_I_H
#define UI_MODEL_ED_OBJ_META_I_H
#include "cpe/pal/pal_queue.h"
#include "render/model_ed/ui_ed_obj.h"

struct ui_ed_rel_meta {
    ui_ed_rel_type_t m_type;
    ui_ed_rel_load_fun_t m_load;
};

struct ui_ed_obj_meta {
    ui_ed_obj_type_t m_type;
    ui_data_src_type_t m_src_type;
    LPDRMETA m_data_meta;
    LPDRMETAENTRY m_id_entry;
    ui_ed_obj_delete_fun_t m_delete;
    ui_ed_obj_init_fun_t m_init;
    ui_ed_obj_set_id_fun_t m_set_id;
    struct ui_ed_rel_meta m_rels[UI_ED_REL_TYPE_MAX - UI_ED_REL_TYPE_MIN];
    ui_ed_obj_create_fun_t m_child_create[UI_ED_OBJ_TYPE_MAX - UI_ED_OBJ_TYPE_MIN];
};

void ui_ed_obj_mgr_init_metas(ui_ed_mgr_t ed_mgr);
void ui_ed_obj_mgr_fini_metas(ui_ed_mgr_t ed_mgr);

#endif
