#include <assert.h>
#include "render/model/ui_data_src.h"
#include "render/model/ui_data_action.h"
#include "ui_ed_src_i.h"
#include "ui_ed_obj_i.h"

int ui_ed_src_load_action(ui_ed_src_t ed_src) {
    ui_data_action_t action;
    struct ui_data_actor_it actor_it;
    ui_data_actor_t actor;
    ui_ed_obj_t action_obj;

    action = ui_data_src_product(ed_src->m_data_src);
    assert(action);

    action_obj = ed_src->m_root_obj,

    ui_data_action_actors(&actor_it, action);
    while((actor = ui_data_actor_it_next(&actor_it))) {
        struct ui_data_actor_layer_it layer_it;
        ui_data_actor_layer_t layer;
        ui_ed_obj_t actor_obj;

        actor_obj =
            ui_ed_obj_create_i(
                ed_src, action_obj,
                ui_ed_obj_type_actor,
                actor, ui_data_actor_data(actor), sizeof(*ui_data_actor_data(actor)));
        if (actor_obj == NULL) {
            return -1;
        }

        ui_data_actor_layers(&layer_it, actor);
        while((layer = ui_data_actor_layer_it_next(&layer_it))) {
            ui_ed_obj_t layer_obj;

            layer_obj =
                ui_ed_obj_create_i(
                    ed_src, actor_obj,
                    ui_ed_obj_type_actor_layer,
                    layer, ui_data_actor_layer_data(layer), sizeof(*ui_data_actor_layer_data(layer)));
            if (layer_obj == NULL) {
                return -1;
            }
        }
    }
    
    return 0;
}

ui_ed_obj_t ui_ed_obj_create_actor(ui_ed_obj_t parent) {
    ui_data_action_t action;
    ui_ed_src_t ed_src = parent->m_src;
    ui_data_actor_t actor;
    ui_ed_obj_t obj;

    action = ui_data_src_product(ed_src->m_data_src);
    assert(action);

    actor = ui_data_actor_create(action);
    if (actor == NULL) return NULL;

    obj = ui_ed_obj_create_i(
        parent->m_src, parent,
        ui_ed_obj_type_actor,
        actor, ui_data_actor_data(actor), sizeof(*ui_data_actor_data(actor)));
    if (obj == NULL) {
        ui_data_actor_free(actor);
        return NULL;
    }

    return obj;
}

ui_ed_obj_t ui_ed_obj_create_actor_layer(ui_ed_obj_t parent) {
    ui_data_actor_t actor;
    ui_ed_src_t ed_src = parent->m_src;
    ui_data_actor_layer_t actor_layer;
    ui_ed_obj_t obj;

    actor = ui_ed_obj_product(parent);
    assert(actor);

    actor_layer = ui_data_actor_layer_create(actor);
    if (actor_layer == NULL) return NULL;

    obj = ui_ed_obj_create_i(
        ed_src, parent,
        ui_ed_obj_type_actor_layer,
        actor_layer, ui_data_actor_layer_data(actor_layer), sizeof(*ui_data_actor_layer_data(actor_layer)));
    if (obj == NULL) {
        ui_data_actor_layer_free(actor_layer);
        return NULL;
    }

    return obj;
}

ui_ed_obj_t ui_ed_obj_create_actor_frame(ui_ed_obj_t parent) {
    ui_data_actor_layer_t actor_layer;
    ui_ed_src_t ed_src = parent->m_src;
    ui_data_actor_frame_t actor_frame;
    ui_ed_obj_t obj;

    actor_layer = ui_ed_obj_product(parent);
    assert(actor_layer);

    actor_frame = ui_data_actor_frame_create(actor_layer);
    if (actor_frame == NULL) return NULL;

    obj = ui_ed_obj_create_i(
        ed_src, parent,
        ui_ed_obj_type_actor_frame,
        actor_frame, ui_data_actor_frame_data(actor_frame), sizeof(*ui_data_actor_frame_data(actor_frame)));
    if (obj == NULL) {
        ui_data_actor_frame_free(actor_frame);
        return NULL;
    }

    return obj;
}

