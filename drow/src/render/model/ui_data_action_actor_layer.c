#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "ui_data_action_i.h"

ui_data_actor_layer_t ui_data_actor_layer_create(ui_data_actor_t actor) {
    ui_data_mgr_t mgr = actor->m_action->m_mgr;
    ui_data_actor_layer_t actor_layer;

    actor_layer = mem_alloc(mgr->m_alloc, sizeof(struct ui_data_actor_layer));
    if (actor_layer == NULL) {
        CPE_ERROR(mgr->m_em, "create actor layer fail !");
        return NULL;
    }

    actor_layer->m_actor = actor;
    bzero(&actor_layer->m_data, sizeof(actor_layer->m_data));

    actor_layer->m_frame_count = 0;
    TAILQ_INIT(&actor_layer->m_frames);

    actor->m_layer_count++;
    TAILQ_INSERT_TAIL(&actor->m_layers, actor_layer, m_next_for_actor);

    return actor_layer;
}

void ui_data_actor_layer_free(ui_data_actor_layer_t actor_layer) {
    ui_data_actor_t actor = actor_layer->m_actor;
    ui_data_mgr_t mgr = actor->m_action->m_mgr;

    while(!TAILQ_EMPTY(&actor_layer->m_frames)) {
        ui_data_actor_frame_free(TAILQ_FIRST(&actor_layer->m_frames));
    }
    assert(actor_layer->m_frame_count == 0);

    actor->m_layer_count--;
    TAILQ_REMOVE(&actor->m_layers, actor_layer, m_next_for_actor);

    mem_free(mgr->m_alloc, actor_layer);
}

static ui_data_actor_layer_t ui_data_actor_layer_in_actor_next(ui_data_actor_layer_it_t it) {
    ui_data_actor_layer_t * data = (ui_data_actor_layer_t *)(it->m_data);
    ui_data_actor_layer_t r;

    if (*data == NULL) return NULL;

    r = *data;

    *data = TAILQ_NEXT(r, m_next_for_actor);

    return r;
}

void ui_data_actor_layers(ui_data_actor_layer_it_t it, ui_data_actor_t actor) {
    *(ui_data_actor_layer_t *)(it->m_data) = TAILQ_FIRST(&actor->m_layers);
    it->next = ui_data_actor_layer_in_actor_next;
}

UI_ACTOR_LAYER * ui_data_actor_layer_data(ui_data_actor_layer_t actor_layer) {
    return &actor_layer->m_data;
}

LPDRMETA ui_data_actor_layer_meta(ui_data_mgr_t mgr) {
    return mgr->m_meta_actor_layer;
}

uint32_t ui_data_actor_layer_frame_count(ui_data_actor_layer_t actor_layer) {
    return actor_layer->m_frame_count;
}

ui_data_actor_layer_t ui_data_actor_layer_get_at(ui_data_actor_t actor, uint32_t idx) {
    ui_data_actor_layer_t layer;

    TAILQ_FOREACH(layer, &actor->m_layers, m_next_for_actor) {
        if (idx == 0) return layer;
        idx--;
    }

    return NULL;
}
