#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "ui_data_action_i.h"
#include "ui_data_src_src_i.h"

ui_data_actor_frame_t ui_data_actor_frame_create(ui_data_actor_layer_t actor_layer) {
    ui_data_mgr_t mgr = actor_layer->m_actor->m_action->m_mgr;
    ui_data_actor_frame_t actor_frame;

    actor_frame = mem_alloc(mgr->m_alloc, sizeof(struct ui_data_actor_frame));
    if (actor_frame == NULL) {
        CPE_ERROR(mgr->m_em, "create actor_frame fail !");
        return NULL;
    }

    actor_frame->m_layer = actor_layer;
    bzero(&actor_frame->m_data, sizeof(actor_frame->m_data));

    actor_layer->m_frame_count++;
    TAILQ_INSERT_TAIL(&actor_layer->m_frames, actor_frame, m_next_for_layer);

    return actor_frame;
}

void ui_data_actor_frame_free(ui_data_actor_frame_t actor_frame) {
    ui_data_actor_layer_t actor_layer = actor_frame->m_layer;
    ui_data_mgr_t mgr = actor_layer->m_actor->m_action->m_mgr;

    actor_layer->m_frame_count--;
    TAILQ_REMOVE(&actor_layer->m_frames, actor_frame, m_next_for_layer);

    mem_free(mgr->m_alloc, actor_frame);
}

static ui_data_actor_frame_t ui_data_actor_frame_in_layer_next(ui_data_actor_frame_it_t it) {
    ui_data_actor_frame_t * data = (ui_data_actor_frame_t *)(it->m_data);
    ui_data_actor_frame_t r;

    if (*data == NULL) return NULL;

    r = *data;

    *data = TAILQ_NEXT(r, m_next_for_layer);

    return r;
}

void ui_data_actor_layer_frames(ui_data_actor_frame_it_t it, ui_data_actor_layer_t layer) {
    *(ui_data_actor_frame_t *)(it->m_data) = TAILQ_FIRST(&layer->m_frames);
    it->next = ui_data_actor_frame_in_layer_next;
}

ui_data_actor_frame_t ui_data_actor_frame_get_at(ui_data_actor_layer_t actor_layer, uint32_t idx) {
    ui_data_actor_frame_t frame;

    TAILQ_FOREACH(frame, &actor_layer->m_frames, m_next_for_layer) {
        if (idx == 0) return frame;
        idx--;
    }

    return NULL;
}

static ui_data_actor_frame_t ui_data_actor_frame_in_actor_layer_next(struct ui_data_actor_frame_it * it) {
    ui_data_actor_frame_t * data = (ui_data_actor_frame_t *)(it->m_data);
    ui_data_actor_frame_t r;

    if (*data == NULL) return NULL;

    r = *data;

    *data = TAILQ_NEXT(r, m_next_for_layer);

    return r;
}

void ui_data_actor_frame_refs(ui_data_actor_frame_it_t it, ui_data_actor_layer_t actor_layer) {
    *(ui_data_actor_frame_t *)(it->m_data) = TAILQ_FIRST(&actor_layer->m_frames);
    it->next = ui_data_actor_frame_in_actor_layer_next;
}

UI_ACTOR_FRAME * ui_data_actor_frame_data(ui_data_actor_frame_t actor_frame) {
    return &actor_frame->m_data;
}

LPDRMETA ui_data_actor_frame_meta(ui_data_mgr_t mgr) {
    return mgr->m_meta_actor_frame;
}
