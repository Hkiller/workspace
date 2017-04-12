#ifndef UI_MODEL_ED_SRC_META_I_H
#define UI_MODEL_ED_SRC_META_I_H
#include "cpe/pal/pal_queue.h"
#include "render/model_ed/ui_ed_obj.h"

struct ui_ed_src_meta {
    ui_ed_src_load_fun_t m_load_fun;
};

void ui_ed_obj_mgr_init_src_metas(ui_ed_mgr_t ed_mgr);

#endif
