#ifndef UI_MODEL_ED_MGR_I_H
#define UI_MODEL_ED_MGR_I_H
#include "cpe/pal/pal_queue.h"
#include "cpe/utils/hash.h"
#include "cpe/utils/buffer.h"
#include "render/model_ed/ui_ed_mgr.h"
#include "ui_ed_obj_meta_i.h"
#include "ui_ed_src_meta_i.h"

typedef TAILQ_HEAD(ui_ed_obj_list, ui_ed_obj) ui_ed_obj_list_t;

struct ui_ed_mgr {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    uint8_t m_debug;
    ui_data_mgr_t m_data_mgr;

    struct mem_buffer m_dump_buffer;

    struct ui_ed_src_meta m_src_metas[UI_DATA_SRC_TYPE_MAX - UI_DATA_SRC_TYPE_MIN];
    struct ui_ed_obj_meta m_metas[UI_ED_OBJ_TYPE_MAX - UI_ED_OBJ_TYPE_MIN];

    struct cpe_hash_table m_ed_srcs;
    struct cpe_hash_table m_ed_objs;
};

#endif
