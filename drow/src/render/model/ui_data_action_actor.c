#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_strings.h"
#include "render/utils/ui_rect.h"
#include "render/model/ui_data_src.h"
#include "render/model/ui_data_module.h"
#include "render/model/ui_data_sprite.h"
#include "ui_data_action_i.h"
#include "ui_data_src_i.h"

ui_data_actor_t ui_data_actor_create(ui_data_action_t action) {
    ui_data_mgr_t mgr = action->m_mgr;
    ui_data_actor_t actor;

    actor = mem_alloc(mgr->m_alloc, sizeof(struct ui_data_actor));
    if (actor == NULL) {
        CPE_ERROR(
            mgr->m_em, "create img in action %s: alloc fail !",
            ui_data_src_path_dump(&mgr->m_dump_buffer, action->m_src));
        return NULL;
    }

    actor->m_action = action;
    bzero(&actor->m_data, sizeof(actor->m_data));
    actor->m_data.id = (uint32_t)-1;

    actor->m_layer_count = 0;
    TAILQ_INIT(&actor->m_layers);

    action->m_actor_count++;
    TAILQ_INSERT_TAIL(&action->m_actors, actor, m_next_for_action);

    return actor;
}

void ui_data_actor_free(ui_data_actor_t actor) {
    ui_data_action_t action = actor->m_action;
    ui_data_mgr_t mgr = action->m_mgr;

    while(!TAILQ_EMPTY(&actor->m_layers)) {
        ui_data_actor_layer_free(TAILQ_FIRST(&actor->m_layers));
    }
    assert(actor->m_layer_count == 0);

    action->m_actor_count--;
    TAILQ_REMOVE(&action->m_actors, actor, m_next_for_action);

    if (actor->m_data.id != (uint32_t)-1) {
        cpe_hash_table_remove_by_ins(&mgr->m_actors, actor);
    }
    
    mem_free(mgr->m_alloc, actor);
}

int ui_data_actor_set_id(ui_data_actor_t actor, uint32_t id) {
    ui_data_mgr_t mgr = actor->m_action->m_mgr;
    uint32_t old_id;

    old_id = actor->m_data.id;

    if (actor->m_data.id != (uint32_t)-1) {
        cpe_hash_table_remove_by_ins(&mgr->m_actors, actor);
    }

    actor->m_data.id = id;

    if (actor->m_data.id != (uint32_t)-1) {
        cpe_hash_entry_init(&actor->m_hh_for_mgr);
        if (cpe_hash_table_insert_unique(&mgr->m_actors, actor) != 0) {
            actor->m_data.id = old_id;
            if (old_id != (uint32_t)-1) {
                cpe_hash_table_insert_unique(&mgr->m_actors, actor);
            }
            return -1;
        }
    }

    return 0;
}

ui_data_actor_t ui_data_actor_find_by_id(ui_data_action_t action, uint32_t id) {
    struct ui_data_actor key;
    key.m_action = action;
    key.m_data.id = id;

    return cpe_hash_table_find(&action->m_mgr->m_actors, &key);
}

ui_data_actor_t ui_data_actor_find_by_name(ui_data_action_t action, const char * name) {
    ui_data_actor_t actor;

    TAILQ_FOREACH(actor, &action->m_actors, m_next_for_action) {
        if (strcmp(actor->m_data.name, name) == 0) return actor;
    }

    return NULL;
}

static ui_data_actor_t ui_data_actor_in_action_next(ui_data_actor_it_t it) {
    ui_data_actor_t * data = (ui_data_actor_t *)(it->m_data);
    ui_data_actor_t r;

    if (*data == NULL) return NULL;

    r = *data;

    *data = TAILQ_NEXT(r, m_next_for_action);

    return r;
}

void ui_data_action_actors(ui_data_actor_it_t it, ui_data_action_t action) {
    *(ui_data_actor_t *)(it->m_data) = TAILQ_FIRST(&action->m_actors);
    it->next = ui_data_actor_in_action_next;
}

UI_ACTOR * ui_data_actor_data(ui_data_actor_t actor) {
    return &actor->m_data;
}

LPDRMETA ui_data_actor_meta(ui_data_mgr_t mgr) {
    return mgr->m_meta_actor;
}

uint32_t ui_data_actor_layer_count(ui_data_actor_t actor) {
    return actor->m_layer_count;
}

uint16_t ui_data_actor_frame_total(ui_data_actor_t actor) {
	if (actor->m_data.time_total) {
		return actor->m_data.time_total;
	}
	else {
        ui_data_actor_layer_t layer;
		uint16_t frame_total = 0;

        TAILQ_FOREACH(layer, &actor->m_layers, m_next_for_actor) {
            ui_data_actor_frame_t last_frame = TAILQ_LAST(&layer->m_frames, ui_data_actor_frame_list);
            if (last_frame && last_frame->m_data.start_frame > frame_total) {
                frame_total = last_frame->m_data.start_frame;
            }
		}

		return frame_total + 1;
	}
}

int ui_data_actor_bounding_rect(ui_data_actor_t actor, ui_rect_t rect) {
    ui_data_mgr_t mgr = actor->m_action->m_mgr;
    ui_data_actor_layer_t layer;
    int rv = 0;
    
    *rect = UI_RECT_ZERO;

    TAILQ_FOREACH(layer, &actor->m_layers, m_next_for_actor) {
        ui_data_actor_frame_t frame;

        TAILQ_FOREACH(frame, &layer->m_frames, m_next_for_layer) {
            ui_rect frame_rect;
            
            switch(frame->m_data.texture.type) {
            case UI_TEXTURE_REF_IMG: {
                ui_data_src_t src;
                ui_data_module_t data_module;
                ui_data_img_block_t img_block;

                src = ui_data_src_find_by_id(mgr, frame->m_data.texture.data.img.module_id);
                if (src == NULL) {
                    CPE_ERROR(mgr->m_em, "ui_data_actor_bounding_rect: src %d not exist", frame->m_data.texture.data.img.module_id);
                    rv = -1;
                    continue;
                }

                data_module = ui_data_src_product(src);
                if (data_module == NULL) {
                    CPE_ERROR(
                        mgr->m_em, "ui_data_actor_bounding_rect: src %s not loaded",
                        ui_data_src_path_dump(gd_app_tmp_buffer(mgr->m_app), src));
                    rv = -1;
                    continue;
                }
                
                img_block = ui_data_img_block_find_by_id(data_module, frame->m_data.texture.data.img.img_block_id);
                if (img_block == NULL) {
                    CPE_ERROR(
                        mgr->m_em, "ui_data_actor_bounding_rect: src %s no img block %d",
                        ui_data_src_path_dump(gd_app_tmp_buffer(mgr->m_app), src), frame->m_data.texture.data.img.img_block_id);
                    return -1;
                }

                ui_data_img_block_bounding_rect(img_block, &frame_rect);
                break;
            }
            case UI_TEXTURE_REF_FRAME: {
                ui_data_src_t src;
                ui_data_sprite_t data_sprite;
                ui_data_frame_t sprite_frame;

                src = ui_data_src_find_by_id(mgr, frame->m_data.texture.data.frame.sprite_id);
                if (src == NULL) {
                    CPE_ERROR(mgr->m_em, "ui_data_actor_bounding_rect: src %d not exist", frame->m_data.texture.data.frame.sprite_id);
                    rv = -1;
                    continue;
                }

                data_sprite = ui_data_src_product(src);
                if (data_sprite == NULL) {
                    CPE_ERROR(
                        mgr->m_em, "ui_data_actor_bounding_rect: src %s not loaded",
                        ui_data_src_path_dump(gd_app_tmp_buffer(mgr->m_app), src));
                    rv = -1;
                    continue;
                }
                
                sprite_frame = ui_data_frame_find_by_id(data_sprite, frame->m_data.texture.data.frame.frame_id);
                if (sprite_frame == NULL) {
                    CPE_ERROR(
                        mgr->m_em, "ui_data_actor_bounding_rect: src %s no frame %d",
                        ui_data_src_path_dump(gd_app_tmp_buffer(mgr->m_app), src), frame->m_data.texture.data.frame.frame_id);
                    return -1;
                }

                ui_data_frame_bounding_rect(sprite_frame, &frame_rect);
                break;
            }
            default:
                CPE_ERROR(
                    mgr->m_em, "ui_data_actor_bounding_rect: uknown ref type %d", frame->m_data.texture.type);
                rv = -1;
            }

            ui_rect_inline_union(rect, &frame_rect);
        }
    }

    return rv;
}

uint32_t ui_data_actor_hash(const ui_data_actor_t actor) {
    return actor->m_action->m_src->m_id & actor->m_data.id;
}

int ui_data_actor_eq(const ui_data_actor_t l, const ui_data_actor_t r) {
    return l->m_data.id == r->m_data.id && l->m_action == r->m_action;
}
