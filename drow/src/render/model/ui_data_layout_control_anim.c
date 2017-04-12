#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dr/dr_data.h"
#include "render/model/ui_data_src.h"
#include "ui_data_layout_i.h"
#include "ui_data_src_i.h"

ui_data_control_anim_t ui_data_control_anim_create(ui_data_control_t control) {
    ui_data_mgr_t mgr = control->m_layout->m_mgr;
    ui_data_control_anim_t anim;

    anim = mem_alloc(mgr->m_alloc, sizeof(struct ui_data_control_anim));
    if (anim == NULL) {
        CPE_ERROR(mgr->m_em, "create anim fail");
        return NULL;
    }

    bzero(anim, sizeof(*anim));
    anim->m_control = control;
    TAILQ_INIT(&anim->m_frames);
    control->m_anim_count++;

    TAILQ_INSERT_TAIL(&control->m_anims, anim, m_next_for_control);
    
    return anim;
}

void ui_data_control_anim_free(ui_data_control_anim_t anim) {
    ui_data_control_t control = anim->m_control;
    ui_data_mgr_t mgr = control->m_layout->m_mgr;

    while(!TAILQ_EMPTY(&anim->m_frames)) {
        ui_data_control_anim_frame_free(TAILQ_FIRST(&anim->m_frames));
    }
    assert(anim->m_anim_frame_count == 0);

    TAILQ_REMOVE(&control->m_anims, anim, m_next_for_control);
    control->m_anim_count--;
    
    mem_free(mgr->m_alloc, anim);
}

ui_data_control_t ui_data_control_anim_control(ui_data_control_anim_t anim) {
    return anim->m_control;
}

LPDRMETA ui_data_control_anim_meta(ui_data_mgr_t mgr) {
    return mgr->m_meta_control_anim;
}

UI_CONTROL_ANIM *
ui_data_control_anim_data(ui_data_control_anim_t anim) {
    return &anim->m_data;
}

uint16_t ui_data_control_anim_count(ui_data_control_t control) {
    return control->m_anim_count;
}

static ui_data_control_anim_t ui_data_control_anim_next(struct ui_data_control_anim_it * it) {
    ui_data_control_anim_t * data = (ui_data_control_anim_t *)(it->m_data);
    ui_data_control_anim_t r;

    if (*data == NULL) return NULL;

    r = *data;

    *data = TAILQ_NEXT(r, m_next_for_control);

    return r;
}

void ui_data_control_anims(ui_data_control_anim_it_t it, ui_data_control_t control) {
    *(ui_data_control_anim_t *)(it->m_data) = TAILQ_FIRST(&control->m_anims);
    it->next = ui_data_control_anim_next;
}

ui_data_control_anim_t ui_data_control_anim_find(ui_data_control_t control, uint8_t type) {
    ui_data_control_anim_t anim;

    TAILQ_FOREACH(anim, &control->m_anims, m_next_for_control) {
        if (anim->m_data.anim_type == type) return anim;
    }

    return NULL;
}
