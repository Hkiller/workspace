#ifndef UI_MODEL_ED_OBJ_REL_I_H
#define UI_MODEL_ED_OBJ_REL_I_H
#include "cpe/pal/pal_queue.h"
#include "render/model_ed/ui_ed_obj.h"

typedef TAILQ_HEAD(ui_ed_rel_list, ui_ed_rel) ui_ed_rel_list_t;

struct ui_ed_rel {
    ui_ed_rel_type_t m_type;

    ui_ed_obj_t m_a_side;
    TAILQ_ENTRY(ui_ed_rel) m_next_for_a_side;

    ui_ed_obj_t m_b_side;
    TAILQ_ENTRY(ui_ed_rel) m_next_for_b_side;
};

ui_ed_rel_t ui_ed_rel_create_i(ui_ed_rel_type_t rel_type, ui_ed_obj_t a_side, ui_ed_obj_t b_side);
void ui_ed_rel_free_i(ui_ed_rel_t ed_rel);
void ui_ed_rel_disconnect(ui_ed_rel_t ed_rel);
void ui_ed_rel_connect(ui_ed_rel_t ed_rel);

#endif
