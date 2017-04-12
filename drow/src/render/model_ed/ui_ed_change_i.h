#ifndef UI_MODEL_ED_CHANGE_I_H
#define UI_MODEL_ED_CHANGE_I_H
#include "cpe/pal/pal_queue.h"
#include "cpe/utils/hash.h"
#include "cpe/utils/buffer.h"
#include "render/model_ed/ui_ed_mgr.h"
#include "ui_ed_obj_meta_i.h"

struct ui_ed_change {
    TAILQ_ENTRY(ui_ed_change) m_next;
};

#endif
