#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "cpe/dr/dr_data.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "ui_ed_obj_i.h"
#include "ui_ed_rel_i.h"

ui_ed_obj_t ui_ed_obj_create_i(
    ui_ed_src_t ed_src, ui_ed_obj_t parent, ui_ed_obj_type_t obj_type,
    void * product, void * data, uint16_t data_capacity)
{
    uint8_t rel_type;
    ui_ed_mgr_t ed_mgr = ed_src->m_ed_mgr;
    ui_ed_obj_t ed_obj;
    ui_ed_obj_meta_t ed_obj_meta;
    ui_ed_rel_t ed_rel;

    assert(obj_type >= UI_ED_OBJ_TYPE_MIN && (uint8_t)obj_type < UI_ED_OBJ_TYPE_MAX);

    ed_obj_meta = &ed_mgr->m_metas[obj_type - UI_ED_OBJ_TYPE_MIN];
    assert(ed_obj_meta);

    ed_obj = mem_alloc(ed_mgr->m_alloc, sizeof(struct ui_ed_obj));
    if (ed_obj == NULL) {
        CPE_ERROR(ed_mgr->m_em, "ed obj create fail!");
        return NULL;
    }

    ed_obj->m_src = ed_src;
    ed_obj->m_id = 0;
    TAILQ_INSERT_TAIL(&ed_src->m_objs, ed_obj, m_next_for_src);

    if (ed_obj_meta->m_id_entry) {
        if (dr_entry_try_read_uint32(
                &ed_obj->m_id, ((char *)data) + dr_entry_data_start_pos(ed_obj_meta->m_id_entry, 0), ed_obj_meta->m_id_entry, NULL)
            != 0)
        {
            CPE_ERROR(ed_mgr->m_em, "ed obj create: read id fail!");
            mem_free(ed_mgr->m_alloc, ed_obj);
            return NULL;
        }

        if (ed_obj->m_id != (uint32_t)-1) {
            cpe_hash_entry_init(&ed_obj->m_hh_for_mgr);
            if (cpe_hash_table_insert(&ed_mgr->m_ed_objs, ed_obj) != 0) {
                CPE_ERROR(ed_mgr->m_em, "ed obj create: insert to ed_objs fail!");
                mem_free(ed_mgr->m_alloc, ed_obj);
                return NULL;
            }
        }
    }

    ed_obj->m_parent = parent;
    TAILQ_INIT(&ed_obj->m_childs);

    ed_obj->m_meta = ed_obj_meta;

    ed_obj->m_product = product;

    ed_obj->m_data = data;
    ed_obj->m_data_capacity = data_capacity;

    TAILQ_INIT(&ed_obj->m_use_objs);
    TAILQ_INIT(&ed_obj->m_used_by_objs);

    for(rel_type = UI_ED_REL_TYPE_MIN; rel_type < UI_ED_REL_TYPE_MAX; ++rel_type) {
        if (ed_obj->m_meta->m_rels[rel_type - UI_ED_REL_TYPE_MIN].m_type != rel_type) continue;

        ed_rel = ui_ed_rel_create_i(rel_type, ed_obj, NULL);
        if (ed_rel == NULL) {
            CPE_ERROR(ed_mgr->m_em, "ed obj create: create rel %s fail!", ui_ed_rel_type_name(rel_type));
            while(!TAILQ_EMPTY(&ed_obj->m_use_objs)) {
                ui_ed_rel_free_i(TAILQ_FIRST(&ed_obj->m_use_objs));
            }
            if (ed_obj_meta->m_id_entry && ed_obj->m_id != (uint32_t)-1) {
                cpe_hash_table_remove_by_ins(&ed_mgr->m_ed_objs, ed_obj);
            }
            mem_free(ed_mgr->m_alloc, ed_obj);
            return NULL;
        }
    }

    if (ed_obj->m_parent) {
        TAILQ_INSERT_TAIL(&ed_obj->m_parent->m_childs, ed_obj, m_next_for_parent);
    }

    return ed_obj;
}

void ui_ed_obj_free_childs_i(ui_ed_obj_t ed_obj) {
    while(!TAILQ_EMPTY(&ed_obj->m_childs)) {
        ui_ed_obj_free_i(TAILQ_FIRST(&ed_obj->m_childs));
    }
}

void ui_ed_obj_free_childs_by_type_i(ui_ed_obj_t ed_obj, ui_ed_obj_type_t obj_type) {
    ui_ed_obj_t child;

    for(child = TAILQ_FIRST(&ed_obj->m_childs); child;) {
        ui_ed_obj_t next = TAILQ_NEXT(child, m_next_for_parent);

        if (child->m_meta->m_type == obj_type) {
            ui_ed_obj_free_i(child);
        }
        else {
            ui_ed_obj_free_childs_by_type_i(child, obj_type);
        }

        child = next;
    }
}

void ui_ed_obj_free_i(ui_ed_obj_t ed_obj) {
    ui_ed_src_t ed_src = ed_obj->m_src;
    ui_ed_mgr_t ed_mgr = ed_src->m_ed_mgr;

    while(!TAILQ_EMPTY(&ed_obj->m_use_objs)) {
        ui_ed_rel_free_i(TAILQ_FIRST(&ed_obj->m_use_objs));
    }

    while(!TAILQ_EMPTY(&ed_obj->m_used_by_objs)) {
        ui_ed_rel_disconnect(TAILQ_FIRST(&ed_obj->m_used_by_objs));
    }
    
    while(!TAILQ_EMPTY(&ed_obj->m_childs)) {
        ui_ed_obj_free_i(TAILQ_FIRST(&ed_obj->m_childs));
    }

    if (ed_obj->m_parent) {
        TAILQ_REMOVE(&ed_obj->m_parent->m_childs, ed_obj, m_next_for_parent);
        ed_obj->m_parent = NULL;
    }

    TAILQ_REMOVE(&ed_src->m_objs, ed_obj, m_next_for_src);
    if (ed_obj->m_meta->m_id_entry && ed_obj->m_id != (uint32_t)-1) {
        cpe_hash_table_remove_by_ins(&ed_mgr->m_ed_objs, ed_obj);
    }

    mem_free(ed_mgr->m_alloc, ed_obj);
}

ui_ed_obj_meta_t ui_ed_obj_meta(ui_ed_obj_t obj) {
    return obj->m_meta;
}

ui_ed_src_t ui_ed_obj_src(ui_ed_obj_t obj) {
    return obj->m_src;
}

void * ui_ed_obj_product(ui_ed_obj_t obj) {
    return obj->m_product;
}

void * ui_ed_obj_data(ui_ed_obj_t obj) {
    return obj->m_data;
}

uint16_t ui_ed_obj_data_capacity(ui_ed_obj_t obj) {
    return obj->m_data_capacity;
}

LPDRMETA ui_ed_obj_data_meta(ui_ed_obj_t obj) {
    return obj->m_meta->m_data_meta;
}

ui_ed_obj_type_t ui_ed_obj_type_id(ui_ed_obj_t obj) {
    return obj->m_meta->m_type;
}

ui_ed_obj_t ui_ed_obj_find_by_id(ui_ed_src_t src, uint32_t id) {
    struct ui_ed_obj key;
    key.m_src = src;
    key.m_id = id;

    return cpe_hash_table_find(&src->m_ed_mgr->m_ed_objs, &key);
}

ui_ed_obj_t ui_ed_obj_parent(ui_ed_obj_t obj) {
    return obj->m_parent;
}

struct ui_ed_obj_child_it_ctx {
    ui_ed_obj_t m_obj;
    uint8_t m_filter;
};

static ui_ed_obj_t ui_ed_obj_childs_next(struct ui_ed_obj_it * it) {
    struct ui_ed_obj_child_it_ctx * data = (struct ui_ed_obj_child_it_ctx *)it->m_data;
    ui_ed_obj_t r;

    if (data->m_obj == NULL) return NULL;

    r = data->m_obj;

    assert(r->m_parent);

    do {
        data->m_obj = TAILQ_NEXT(data->m_obj, m_next_for_parent);
    } while(data->m_obj
            && (data->m_filter != 0 && (uint8_t)ui_ed_obj_type_id(data->m_obj) != data->m_filter));
    
    return r;
}

void ui_ed_obj_childs(ui_ed_obj_it_t it, ui_ed_obj_t obj) {
    struct ui_ed_obj_child_it_ctx * data = (struct ui_ed_obj_child_it_ctx *)it->m_data;
    data->m_obj = TAILQ_FIRST(&obj->m_childs);
    data->m_filter = 0;
    it->next = ui_ed_obj_childs_next;
}

void ui_ed_obj_childs_by_type(ui_ed_obj_it_t it, ui_ed_obj_t obj, ui_ed_obj_type_t obj_type) {
    struct ui_ed_obj_child_it_ctx * data = (struct ui_ed_obj_child_it_ctx *)it->m_data;
    data->m_obj = TAILQ_FIRST(&obj->m_childs);
    data->m_filter = (uint8_t)obj_type;
    it->next = ui_ed_obj_childs_next;
}

ui_ed_obj_t ui_ed_obj_only_child(ui_ed_obj_t obj) {
    ui_ed_obj_t r = TAILQ_FIRST(&obj->m_childs);

    if (r && TAILQ_NEXT(r, m_next_for_parent) == TAILQ_END(&obj->m_childs)) {
        return r;
    }

    return NULL;
}

ui_ed_obj_t ui_ed_obj_first_child_of_type(ui_ed_obj_t obj, ui_ed_obj_type_t obj_type) {
    ui_ed_obj_t r;

    TAILQ_FOREACH(r, &obj->m_childs, m_next_for_parent) {
        if (r->m_meta->m_type == obj_type) return r;
    }

    return NULL;
}

ui_ed_obj_t ui_ed_obj_n_child_of_type(ui_ed_obj_t obj, ui_ed_obj_type_t obj_type, uint32_t n) {
    ui_ed_obj_t r;

    TAILQ_FOREACH(r, &obj->m_childs, m_next_for_parent) {
        if (r->m_meta->m_type == obj_type) {
            if (n > 0) {
                --n;
            }
            else {
                return r;
            }
        }
    }

    return NULL;
}

struct ui_ed_obj_usings_it_data {
    ui_ed_rel_t m_rel;
    ui_ed_rel_type_t m_require_type;
};

static ui_ed_obj_t ui_ed_obj_usings_next(struct ui_ed_obj_it * it) {
    struct ui_ed_obj_usings_it_data * data = (struct ui_ed_obj_usings_it_data *)(it->m_data);
    ui_ed_obj_t r;

    if (data->m_rel == NULL) return NULL;

    r = data->m_rel->m_b_side;

    for(data->m_rel = TAILQ_NEXT(data->m_rel, m_next_for_a_side);
        data->m_rel;
        data->m_rel = TAILQ_NEXT(data->m_rel, m_next_for_a_side))
    {
        if (data->m_require_type == 0 || data->m_rel->m_type == data->m_require_type) break;
    }

    if (data->m_rel && data->m_rel->m_b_side == NULL) {
        ui_ed_rel_connect(data->m_rel);
    }

    return r;
}

void ui_ed_obj_all_usings(ui_ed_obj_it_t obj_it, ui_ed_obj_t obj) {
    struct ui_ed_obj_usings_it_data * data = (struct ui_ed_obj_usings_it_data *)(obj_it->m_data);
    data->m_rel = TAILQ_FIRST(&obj->m_use_objs);
    data->m_require_type = 0;
    obj_it->next = ui_ed_obj_usings_next;
}

void ui_ed_obj_usings(ui_ed_obj_it_t obj_it, ui_ed_obj_t obj, ui_ed_rel_type_t rel_type) {
    struct ui_ed_obj_usings_it_data * data = (struct ui_ed_obj_usings_it_data *)(obj_it->m_data);
    data->m_rel = TAILQ_FIRST(&obj->m_use_objs);
    data->m_require_type = rel_type;
    obj_it->next = ui_ed_obj_usings_next;
    
    if (rel_type && data->m_rel && data->m_rel->m_type != data->m_require_type) {
        ui_ed_obj_it_next(obj_it);
    }
    else if (data->m_rel->m_b_side == NULL) {
        ui_ed_rel_connect(data->m_rel);
    }
}

ui_ed_obj_t ui_ed_obj_using_find(ui_ed_obj_t obj, ui_ed_rel_type_t rel_type) {
    ui_ed_rel_t rel;

    TAILQ_FOREACH(rel, &obj->m_use_objs, m_next_for_a_side) {
        if (rel->m_type == rel_type) {
            if (rel->m_b_side == NULL) {
                ui_ed_rel_connect(rel);
            }
            return rel->m_b_side;
        }
    }

    return NULL;
}

uint32_t ui_ed_obj_hash(ui_ed_obj_t obj) {
    return (obj->m_src->m_data.src_id << 8) | (obj->m_id & 0xF);
}

int ui_ed_obj_eq(ui_ed_obj_t l, ui_ed_obj_t r) {
    return l->m_src == r->m_src && l->m_id == r->m_id;
}

const char * ui_ed_obj_type_name(ui_ed_obj_type_t obj_type) {
    switch(obj_type) {
    case ui_ed_obj_type_src:
        return "src";
    case ui_ed_obj_type_img_block:
        return "img_block";
    case ui_ed_obj_type_frame:
        return "frame";
    case ui_ed_obj_type_frame_img:
        return "frame_img";
    case ui_ed_obj_type_actor:
        return "actor";
    case ui_ed_obj_type_actor_layer:
        return "actor_layer";
    case ui_ed_obj_type_actor_frame:
        return "actor_frame";
    case ui_ed_obj_type_particle_emitter:
        return "particle_emitter";
    case ui_ed_obj_type_particle_mod:
        return "particle_mode";
    case ui_ed_obj_type_barrage:
        return "barrage";
    case ui_ed_obj_type_barrage_emitter:
        return "barrage_emitter";
    case ui_ed_obj_type_barrage_emitter_trigger:
        return "barrage_emitter_trigger";
    case ui_ed_obj_type_barrage_bullet_trigger:
        return "barrage_bullet_trigger";
    case ui_ed_obj_type_moving_plan:
        return "moving_plan";
    case ui_ed_obj_type_moving_track:
        return "moving_track";
    case ui_ed_obj_type_moving_point:
        return "moving_point";
    case ui_ed_obj_type_moving_node:
        return "moving_node";
    case ui_ed_obj_type_moving_segment:
        return "moving_segment";
    case ui_ed_obj_type_chipmunk_scene:
        return "chipmunk_scene";
    case ui_ed_obj_type_chipmunk_body:
        return "chipmunk_body";
    case ui_ed_obj_type_chipmunk_fixture:
        return "chipmunk_fixture";
    case ui_ed_obj_type_chipmunk_polygon_node:
        return "chipmunk_polygon_node";
    default:
        return "unknown-obj-type";
    }
}

ui_ed_obj_type_t ui_ed_obj_type_from_name(const char * obj_type_name) {
    if (strcmp(obj_type_name, "src") == 0) {
        return ui_ed_obj_type_src;
    }
    else if (strcmp(obj_type_name, "img_block") == 0) {
        return ui_ed_obj_type_img_block;
    }
    else if (strcmp(obj_type_name, "frame") == 0) {
        return ui_ed_obj_type_frame;
    }
    else if (strcmp(obj_type_name, "frame_img") == 0) {
        return ui_ed_obj_type_frame_img;
    }
    else if (strcmp(obj_type_name, "actor") == 0) {
        return ui_ed_obj_type_actor;
    }
    else if (strcmp(obj_type_name, "actor_layer") == 0) {
        return ui_ed_obj_type_actor_layer;
    }
    else if (strcmp(obj_type_name, "actor_frame") == 0) {
        return ui_ed_obj_type_actor_frame;
    }
    else if (strcmp(obj_type_name, "particle_emitter") == 0) {
        return ui_ed_obj_type_particle_emitter;
    }
    else if (strcmp(obj_type_name, "particle_mode") == 0) {
        return ui_ed_obj_type_particle_mod;
    }
    else if (strcmp(obj_type_name, "barrage") == 0) {
        return ui_ed_obj_type_barrage;
    }
    else if (strcmp(obj_type_name, "barrage_emitter") == 0) {
        return ui_ed_obj_type_barrage_emitter;
    }
    else if (strcmp(obj_type_name, "barrage_emitter_trigger") == 0) {
        return ui_ed_obj_type_barrage_emitter_trigger;
    }
    else if (strcmp(obj_type_name, "barrage_bullet_trigger") == 0) {
        return ui_ed_obj_type_barrage_bullet_trigger;
    }
    else if (strcmp(obj_type_name, "moving_plan") == 0) {
        return ui_ed_obj_type_moving_plan;
    }
    else if (strcmp(obj_type_name, "moving_track") == 0) {
        return ui_ed_obj_type_moving_track;
    }
    else if (strcmp(obj_type_name, "moving_point") == 0) {
        return ui_ed_obj_type_moving_point;
    }
    else if (strcmp(obj_type_name, "moving_node") == 0) {
        return ui_ed_obj_type_moving_node;
    }
    else if (strcmp(obj_type_name, "moving_segment") == 0) {
        return ui_ed_obj_type_moving_segment;
    }
    else if (strcmp(obj_type_name, "chipmunk_scene") == 0) {
        return ui_ed_obj_type_chipmunk_scene;
    }
    else if (strcmp(obj_type_name, "chipmunk_body") == 0) {
        return ui_ed_obj_type_chipmunk_body;
    }
    else if (strcmp(obj_type_name, "chipmunk_fixture") == 0) {
        return ui_ed_obj_type_chipmunk_fixture;
    }
    else if (strcmp(obj_type_name, "chipmunk_polygon_node") == 0) {
        return ui_ed_obj_type_chipmunk_polygon_node;
    }
    else if (strcmp(obj_type_name, "scrollmap_tile") == 0) {
        return ui_ed_obj_type_scrollmap_tile;
    }
    else if (strcmp(obj_type_name, "scrollmap_layer") == 0) {
        return ui_ed_obj_type_scrollmap_layer;
    }
    else if (strcmp(obj_type_name, "scrollmap_block") == 0) {
        return ui_ed_obj_type_scrollmap_block;
    }
    else if (strcmp(obj_type_name, "scrollmap_script") == 0) {
        return ui_ed_obj_type_scrollmap_script;
    }
    else {
        return 0;
    }
}

ui_ed_obj_t ui_ed_obj_new(ui_ed_obj_t parent, ui_ed_obj_type_t obj_type) {
    ui_ed_src_t ed_src = parent->m_src;
    ui_ed_mgr_t ed_mgr = ed_src->m_ed_mgr;
    ui_ed_obj_create_fun_t fun;
    ui_ed_obj_t r;

    assert(parent);
    assert(obj_type >= UI_ED_OBJ_TYPE_MIN && (uint8_t)obj_type < UI_ED_OBJ_TYPE_MAX);

    fun = parent->m_meta->m_child_create[obj_type - UI_ED_OBJ_TYPE_MIN];

    if (fun == NULL) {
        CPE_ERROR(
            ed_mgr->m_em, "ui_ed_obj_new: can`t create obj %s at %s!",
            ui_ed_obj_type_name(obj_type), ui_ed_obj_type_name(parent->m_meta->m_type));
        return NULL;
    }

    r = fun(parent);

    if (r) {
        dr_meta_set_defaults(r->m_data, r->m_data_capacity, r->m_meta->m_data_meta, DR_SET_DEFAULTS_POLICY_NO_DEFAULT_IGNORE);
    }

    if (r && ed_src->m_state == ui_ed_src_state_normal) {
        ed_src->m_state = ui_ed_src_state_changed;
    }

    return r;
}

void ui_ed_obj_remove(ui_ed_obj_t ed_obj) {
    ui_ed_obj_remove_childs(ed_obj);

    if (ed_obj->m_meta->m_delete) {
        ed_obj->m_meta->m_delete(ed_obj->m_product);
    }

    if (ed_obj->m_src->m_state == ui_ed_src_state_normal) {
        ed_obj->m_src->m_state = ui_ed_src_state_changed;
    }

    ui_ed_obj_free_i(ed_obj);
}

void ui_ed_obj_remove_childs(ui_ed_obj_t ed_obj) {
    while(!TAILQ_EMPTY(&ed_obj->m_childs)) {
        ui_ed_obj_remove(TAILQ_FIRST(&ed_obj->m_childs));
    }
}

void ui_ed_obj_remove_childs_by_type(ui_ed_obj_t ed_obj, ui_ed_obj_type_t obj_type) {
    ui_ed_obj_t child, next_child;
    for(child = TAILQ_FIRST(&ed_obj->m_childs); child; child = next_child) {
        next_child = TAILQ_NEXT(child, m_next_for_parent);
        if (child->m_meta->m_type == obj_type) {
            ui_ed_obj_remove(child);
        }
    }
}

int ui_ed_obj_set_id(ui_ed_obj_t ed_obj, uint32_t id) {
    ui_ed_mgr_t ed_mgr = ed_obj->m_src->m_ed_mgr;
    uint32_t old_id;

    if (ed_obj->m_meta->m_id_entry == NULL) {
        CPE_ERROR(ed_mgr->m_em, "ed obj meta no id entry!");
        return -1;
    }

    if (ed_obj->m_meta->m_set_id == NULL) {
        CPE_ERROR(ed_mgr->m_em, "ed obj can`t set id!");
        return -1;
    }

    old_id = ed_obj->m_id;
    if (old_id != (uint32_t)-1) {
        cpe_hash_table_remove_by_ins(&ed_mgr->m_ed_objs, ed_obj);
    }

    ed_obj->m_id = id;
    if (ed_obj->m_id != (uint32_t)-1) {
        cpe_hash_entry_init(&ed_obj->m_hh_for_mgr);
        if (cpe_hash_table_insert(&ed_mgr->m_ed_objs, ed_obj) != 0) {
            CPE_ERROR(ed_mgr->m_em, "ed obj set id: insert to ed_objs fail!");
            ed_obj->m_id = old_id;
            if (ed_obj->m_id != (uint32_t)-1) {
                cpe_hash_table_insert(&ed_mgr->m_ed_objs, ed_obj);
            }
            return -1;
        }
    }

    if (ed_obj->m_meta->m_set_id(ed_obj->m_product, id) != 0) {
        CPE_ERROR(ed_mgr->m_em, "ed obj %d set id: inner set id %d fail!", ui_ed_src_id(ed_obj->m_src), id);

        if (ed_obj->m_id != (uint32_t)-1) {
            cpe_hash_table_remove_by_ins(&ed_mgr->m_ed_objs, ed_obj);
        }
        
        ed_obj->m_id = old_id;
        
        if (ed_obj->m_id != (uint32_t)-1) {
            cpe_hash_table_insert(&ed_mgr->m_ed_objs, ed_obj);
        }
        return -1;
    }

    return 0;
}
