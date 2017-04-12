#ifndef UI_MODEL_ED_SRC_I_H
#define UI_MODEL_ED_SRC_I_H
#include "cpe/pal/pal_queue.h"
#include "render/model_ed/ui_ed_src.h"
#include "ui_ed_mgr_i.h"

struct ui_ed_src {
    ui_ed_mgr_t m_ed_mgr;
    ui_data_src_t m_data_src;
    ui_ed_obj_t m_root_obj;
    ui_ed_obj_list_t m_objs;
    ui_ed_src_state_t m_state;
    UI_ED_SRC m_data;

    struct cpe_hash_entry m_hh_for_mgr;
};

ui_ed_src_t ui_ed_src_create_i(ui_ed_mgr_t ed_mgr, ui_data_src_t src, ui_ed_src_state_t state);
void ui_ed_src_free_i(ui_ed_src_t ed_src);

void ui_ed_src_free_all(ui_ed_mgr_t ed_mgr);

uint32_t ui_ed_src_hash(ui_ed_src_t src);
int ui_ed_src_eq(ui_ed_src_t l, ui_ed_src_t r);

#endif
