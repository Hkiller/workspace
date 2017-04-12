#include "cpe/pal/pal_strings.h"
#include "ui_ed_src_meta_i.h"
#include "ui_ed_mgr_i.h"

extern int ui_ed_src_load_module(ui_ed_src_t src);
extern int ui_ed_src_load_sprite(ui_ed_src_t src);
extern int ui_ed_src_load_action(ui_ed_src_t src);
extern int ui_ed_src_load_layout(ui_ed_src_t src);

void ui_ed_obj_mgr_init_src_metas(ui_ed_mgr_t ed_mgr) {
    struct ui_ed_src_meta * src_meta;

    bzero(ed_mgr->m_src_metas, sizeof(ed_mgr->m_src_metas));

    src_meta = ed_mgr->m_src_metas + ui_data_src_type_module - UI_DATA_SRC_TYPE_MIN;
    src_meta->m_load_fun = ui_ed_src_load_module;

    src_meta = ed_mgr->m_src_metas + ui_data_src_type_sprite - UI_DATA_SRC_TYPE_MIN;
    src_meta->m_load_fun = ui_ed_src_load_sprite;

    src_meta = ed_mgr->m_src_metas + ui_data_src_type_action - UI_DATA_SRC_TYPE_MIN;
    src_meta->m_load_fun = ui_ed_src_load_action;

    src_meta = ed_mgr->m_src_metas + ui_data_src_type_layout - UI_DATA_SRC_TYPE_MIN;
    src_meta->m_load_fun = ui_ed_src_load_layout;
}
