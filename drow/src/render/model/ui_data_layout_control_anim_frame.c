#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dr/dr_data.h"
#include "render/model/ui_data_src.h"
#include "ui_data_layout_i.h"
#include "ui_data_src_i.h"

ui_data_control_anim_frame_t ui_data_control_anim_frame_create(ui_data_control_anim_t anim) {
    ui_data_mgr_t mgr = anim->m_control->m_layout->m_mgr;
    ui_data_control_anim_frame_t anim_frame;

    anim_frame = mem_alloc(mgr->m_alloc, sizeof(struct ui_data_control_anim_frame));
    if (anim_frame == NULL) {
        CPE_ERROR(mgr->m_em, "create anim_frame fail");
        return NULL;
    }

    bzero(anim_frame, sizeof(*anim_frame));
    anim_frame->m_anim = anim;
    anim->m_anim_frame_count++;
    
    TAILQ_INSERT_TAIL(&anim->m_frames, anim_frame, m_next_for_anim);
    
    return anim_frame;
}

void ui_data_control_anim_frame_free(ui_data_control_anim_frame_t anim_frame) {
    ui_data_control_anim_t anim = anim_frame->m_anim;
    ui_data_mgr_t mgr = anim->m_control->m_layout->m_mgr;

    anim->m_anim_frame_count--;
    TAILQ_REMOVE(&anim->m_frames, anim_frame, m_next_for_anim);
    
    mem_free(mgr->m_alloc, anim_frame);
}

ui_data_control_anim_t ui_data_control_anim_frame_anim(ui_data_control_anim_frame_t anim_frame) {
    return anim_frame->m_anim;
}

LPDRMETA ui_data_control_anim_frame_meta(ui_data_mgr_t mgr) {
    return mgr->m_meta_control_anim_frame;
}

UI_CONTROL_ANIM_FRAME *
ui_data_control_anim_frame_data(ui_data_control_anim_frame_t anim_frame) {
    return &anim_frame->m_data;
}

uint16_t ui_data_control_anim_frame_count(ui_data_control_anim_t anim) {
    return anim->m_anim_frame_count;
}

static ui_data_control_anim_frame_t ui_data_control_anim_frame_it_do_next(struct ui_data_control_anim_frame_it * it) {
    ui_data_control_anim_frame_t * data = (ui_data_control_anim_frame_t *)(it->m_data);
    ui_data_control_anim_frame_t r;

    if (*data == NULL) return NULL;

    r = *data;

    *data = TAILQ_NEXT(r, m_next_for_anim);

    return r;
}

void ui_data_control_anim_frames(ui_data_control_anim_frame_it_t it, ui_data_control_anim_t anim) {
    *(ui_data_control_anim_frame_t *)(it->m_data) = TAILQ_FIRST(&anim->m_frames);
    it->next = ui_data_control_anim_frame_it_do_next;
}

ui_data_control_anim_frame_t ui_data_control_anim_frame_next(ui_data_control_anim_frame_t anim_frame) {
    return anim_frame ? TAILQ_NEXT(anim_frame, m_next_for_anim) : NULL;
}
