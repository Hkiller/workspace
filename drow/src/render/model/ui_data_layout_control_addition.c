#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dr/dr_data.h"
#include "render/model/ui_data_src.h"
#include "ui_data_layout_i.h"
#include "ui_data_src_i.h"

ui_data_control_addition_t ui_data_control_addition_create(ui_data_control_t control) {
    ui_data_mgr_t mgr = control->m_layout->m_mgr;
    ui_data_control_addition_t addition;

    addition = mem_alloc(mgr->m_alloc, sizeof(struct ui_data_control_addition));
    if (addition == NULL) {
        CPE_ERROR(mgr->m_em, "create addition fail");
        return NULL;
    }

    bzero(addition, sizeof(*addition));
    addition->m_control = control;
    control->m_addition_count++;

    TAILQ_INSERT_TAIL(&control->m_additions, addition, m_next_for_control);
    
    return addition;
}

void ui_data_control_addition_free(ui_data_control_addition_t addition) {
    ui_data_control_t control = addition->m_control;
    ui_data_mgr_t mgr = control->m_layout->m_mgr;

    TAILQ_REMOVE(&control->m_additions, addition, m_next_for_control);
    control->m_addition_count--;
    
    mem_free(mgr->m_alloc, addition);
}

ui_data_control_t ui_data_control_addition_control(ui_data_control_addition_t addition) {
    return addition->m_control;
}

LPDRMETA ui_data_control_addition_meta(ui_data_mgr_t mgr) {
    return mgr->m_meta_control_addition;
}

UI_CONTROL_ADDITION *
ui_data_control_addition_data(ui_data_control_addition_t addition) {
    return &addition->m_data;
}

uint16_t ui_data_control_addition_count(ui_data_control_t control) {
    return control->m_addition_count;
}

static ui_data_control_addition_t ui_data_control_addition_next(struct ui_data_control_addition_it * it) {
    ui_data_control_addition_t * data = (ui_data_control_addition_t *)(it->m_data);
    ui_data_control_addition_t r;

    if (*data == NULL) return NULL;

    r = *data;

    *data = TAILQ_NEXT(r, m_next_for_control);

    return r;
}

void ui_data_control_additions(ui_data_control_addition_it_t it, ui_data_control_t control) {
    *(ui_data_control_addition_t *)(it->m_data) = TAILQ_FIRST(&control->m_additions);
    it->next = ui_data_control_addition_next;
}

