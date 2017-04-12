#include <assert.h>
#include "ui_data_action_i.h"
#include "ui_data_src_src_i.h"

ui_data_action_t ui_data_action_create(ui_data_mgr_t mgr, ui_data_src_t src) {
    ui_data_action_t action;

    if (src->m_type != ui_data_src_type_action) {
        CPE_ERROR(
            mgr->m_em, "create action at %s: src not action!",
            ui_data_src_path_dump(&mgr->m_dump_buffer, src));
        return NULL;
    }

    if (src->m_product) {
        CPE_ERROR(
            mgr->m_em, "create action at %s: product already loaded!",
            ui_data_src_path_dump(&mgr->m_dump_buffer, src));
        return NULL;
    }

    action = mem_alloc(mgr->m_alloc, sizeof(struct ui_data_action));
    if (action == NULL) {
        CPE_ERROR(
            mgr->m_em, "create action at %s: alloc fail!",
            ui_data_src_path_dump(&mgr->m_dump_buffer, src));
        return NULL;
    }

    action->m_mgr = mgr;
    action->m_src = src;
    action->m_actor_count = 0;

    TAILQ_INIT(&action->m_actors);

    src->m_product = action;

    return action;
}

void ui_data_action_free(ui_data_action_t action) {
    ui_data_mgr_t mgr = action->m_mgr;

    while(!TAILQ_EMPTY(&action->m_actors)) {
        ui_data_actor_free(TAILQ_FIRST(&action->m_actors));
    }
    assert(action->m_actor_count == 0);
    
    assert(action->m_src->m_product == action);
    action->m_src->m_product = NULL;

    mem_free(mgr->m_alloc, action);
}

uint32_t ui_data_action_actor_count(ui_data_action_t action) {
    return action->m_actor_count;
}

int ui_data_action_update_using(ui_data_src_t user) {
    ui_data_action_t action = ui_data_src_product(user);
    ui_data_actor_t actor;
    int rv = 0;

    TAILQ_FOREACH(actor, &action->m_actors, m_next_for_action) {
        ui_data_actor_layer_t layer;

        TAILQ_FOREACH(layer, &actor->m_layers, m_next_for_actor) {
            ui_data_actor_frame_t frame;

            TAILQ_FOREACH(frame, &layer->m_frames, m_next_for_layer) {
                switch(frame->m_data.texture.type) {
                case UI_TEXTURE_REF_IMG:
                    if (ui_data_src_src_create_by_id(user, frame->m_data.texture.data.img.module_id, ui_data_src_type_module) == NULL) rv = -1;
                    break;
                case UI_TEXTURE_REF_FRAME:
                    if (ui_data_src_src_create_by_id(user, frame->m_data.texture.data.frame.sprite_id, ui_data_src_type_sprite) == NULL) rv = -1;
                    break;
                default:
                    CPE_ERROR(action->m_mgr->m_em, "unknown texture ref type %d", frame->m_data.texture.type);
                    rv = -1;
                }

                if (frame->m_data.sound[0]) {
                }

                if (frame->m_data.particle[0]) {
                }

                if (frame->m_data.event[0]) {
                }
            }
        }
    }

    return rv;
}
