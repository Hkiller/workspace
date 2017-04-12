#ifndef UI_MODEL_ED_OBJ_I_H
#define UI_MODEL_ED_OBJ_I_H
#include "cpe/pal/pal_queue.h"
#include "render/model_ed/ui_ed_obj.h"
#include "ui_ed_mgr_i.h"
#include "ui_ed_src_i.h"
#include "ui_ed_obj_meta_i.h"
#include "ui_ed_rel_i.h"

struct ui_ed_obj {
    ui_ed_src_t m_src;
    uint32_t m_id;
    TAILQ_ENTRY(ui_ed_obj) m_next_for_src;
    struct cpe_hash_entry m_hh_for_mgr;

    ui_ed_obj_t m_parent;
    ui_ed_obj_list_t m_childs;
    TAILQ_ENTRY(ui_ed_obj) m_next_for_parent;

    ui_ed_rel_list_t m_use_objs;
    ui_ed_rel_list_t m_used_by_objs;

    void * m_product;

    ui_ed_obj_meta_t m_meta;
    uint16_t m_data_capacity;
    void * m_data;
};

ui_ed_obj_t ui_ed_obj_create_i(ui_ed_src_t ed_src, ui_ed_obj_t parent, ui_ed_obj_type_t obj_type, void * product, void * data, uint16_t data_capacity);
void ui_ed_obj_free_i(ui_ed_obj_t ed_obj);
void ui_ed_obj_free_childs_i(ui_ed_obj_t ed_obj);
void ui_ed_obj_free_childs_by_type_i(ui_ed_obj_t ed_obj, ui_ed_obj_type_t obj_type);

uint32_t ui_ed_obj_hash(ui_ed_obj_t src);
int ui_ed_obj_eq(ui_ed_obj_t l, ui_ed_obj_t r);

#endif
