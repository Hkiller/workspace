#include <assert.h>
#include "cpe/pal/pal_stdlib.h"
#include "cpe/utils/string_utils.h"
#include "cpe/utils/math_ex.h"
#include "render/utils/ui_quaternion.h"
#include "render/runtime/ui_runtime_render_obj.h"
#include "render/runtime/ui_runtime_render_obj_ref.h"
#include "ui/sprite/ui_sprite_world.h"
#include "ui/sprite/ui_sprite_world_res.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite/ui_sprite_component.h"
#include "ui/sprite_2d/ui_sprite_2d_transform.h"
#include "ui_sprite_render_anim_i.h"
#include "ui_sprite_render_group_i.h"

static void ui_runtime_render_anim_on_event(void * ctx, ui_runtime_render_obj_t obj, const char * evt);

ui_sprite_render_anim_t
ui_sprite_render_anim_create_i(
    ui_sprite_render_layer_t layer, ui_runtime_render_obj_ref_t render_obj_ref, ui_sprite_render_group_t group, const char * name)
{
    ui_sprite_render_env_t env = layer->m_env;
    ui_sprite_render_anim_t anim;

    assert(render_obj_ref);

    anim = TAILQ_FIRST(&env->m_free_anims);
    if (anim) {
        TAILQ_REMOVE(&env->m_free_anims, anim, m_next_for_layer);
    }
    else {
        anim = mem_alloc(env->m_module->m_alloc, sizeof(struct ui_sprite_render_anim));
        if (anim == NULL) {
            CPE_ERROR(env->m_module->m_em, "ui_sprite_render_anim_create_before: alloc fail!");
            return NULL;
        }
    }

    anim->m_layer = layer;
    anim->m_anim_id = env->m_max_id + 1;
    anim->m_render_obj_ref = render_obj_ref;
    anim->m_sync_transform = 0;
    anim->m_auto_remove = 0;
    
    if (name) {
        cpe_str_dup(anim->m_anim_name, sizeof(anim->m_anim_name), name);
    }
    else {
        anim->m_anim_name[0] = 0;
    }
    
    if (group) {
        anim->m_sch = group->m_sch;
        anim->m_group = group;
        anim->m_priority = group->m_sch->m_render_priority + group->m_adj_render_priority;
        anim->m_transform = group->m_world_trans;

        TAILQ_INSERT_TAIL(&group->m_anims, anim, m_next_for_group);
        TAILQ_INSERT_TAIL(&anim->m_sch->m_anims, anim, m_next_for_sch);
        anim->m_sch->m_is_dirty = 1;
    }
    else {
        anim->m_sch = NULL;
        anim->m_group = NULL;
        anim->m_priority = 0.0f;
        anim->m_transform = UI_TRANSFORM_IDENTITY;
        TAILQ_INSERT_TAIL(&env->m_global_anims, anim, m_next_for_group);
    }
    
    cpe_hash_entry_init(&anim->m_hh);
    if (cpe_hash_table_insert(&env->m_anims, anim) != 0) {
        CPE_ERROR(env->m_module->m_em, "ui_sprite_render_anim_create: anim %d duplicate!", anim->m_anim_id);

        if (anim->m_sch) {
            TAILQ_REMOVE(&anim->m_sch->m_anims, anim, m_next_for_sch);
        }
        
        anim->m_layer = (void*)env;
        TAILQ_INSERT_TAIL(&env->m_free_anims, anim, m_next_for_layer);
        return NULL;
    }

    TAILQ_INSERT_HEAD(&layer->m_anims, anim, m_next_for_layer);
    layer->m_is_dirty = 1;
    
    ui_runtime_render_obj_ref_set_evt_processor(anim->m_render_obj_ref, ui_runtime_render_anim_on_event, anim);

    env->m_max_id++;

    return anim;
}

ui_sprite_render_anim_t
ui_sprite_render_anim_create(
    ui_sprite_render_layer_t layer, ui_runtime_render_obj_ref_t render_obj_ref, ui_sprite_render_group_t group, const char * name)
{
    ui_sprite_render_anim_t anim = ui_sprite_render_anim_create_i(layer, render_obj_ref, group, name);
    if (anim) {
        ui_sprite_render_anim_set_sync_transform(anim, 1);
    }
    return anim;
}

ui_sprite_render_anim_t
ui_sprite_render_anim_create_by_res(ui_sprite_render_layer_t layer, const char * res, ui_sprite_render_group_t group, const char * name) {
    ui_sprite_render_env_t env = layer->m_env;
    ui_sprite_render_anim_t anim;
    ui_runtime_render_obj_ref_t render_obj_ref;
    uint32_t entity_id = group ? ui_sprite_render_sch_entity_id(group->m_sch) : 0;
    char * left_args;
    
    render_obj_ref = ui_sprite_render_module_create_obj(
        env->m_module,
        ui_sprite_world_res_world(ui_sprite_world_res_from_data(env)), entity_id, res, &left_args);
    if (render_obj_ref == NULL) return NULL;

    if (name == NULL && left_args) {
        name = cpe_str_read_and_remove_arg(left_args, "anim-name", ',', '=');
    }
    
    anim = ui_sprite_render_anim_create_i(layer, render_obj_ref, group, name);
    if (anim == NULL) {
        ui_runtime_render_obj_ref_free(render_obj_ref);
        return NULL;
    }

    if (left_args) {
        char * str_value;

        if ((str_value = cpe_str_read_and_remove_arg(left_args, "sync-transform", ',', '='))) {
            ui_sprite_render_anim_set_sync_transform(anim, atoi(str_value));
        }
        else {
            ui_sprite_render_anim_set_sync_transform(anim, 1);
        }

        if ((str_value = cpe_str_read_and_remove_arg(left_args, "auto-remove", ',', '='))) {
            ui_sprite_render_anim_set_auto_remove(anim, atoi(str_value));
        }
    }
    else {
        ui_sprite_render_anim_set_sync_transform(anim, 1);
    }
    
    return anim;
}

void ui_sprite_render_anim_free(ui_sprite_render_anim_t anim) {
    ui_sprite_render_layer_t layer = anim->m_layer;
    ui_sprite_render_env_t env = layer->m_env;

    assert(anim->m_render_obj_ref);
    ui_runtime_render_obj_ref_free(anim->m_render_obj_ref);
    anim->m_render_obj_ref = NULL;

    cpe_hash_table_remove_by_ins(&env->m_anims, anim);
    
    TAILQ_REMOVE(&layer->m_anims, anim, m_next_for_layer);

    if (anim->m_group) {
        TAILQ_REMOVE(&anim->m_group->m_anims, anim, m_next_for_group);
    }
    else {
        TAILQ_REMOVE(&env->m_global_anims, anim, m_next_for_group);
    }        
    
    if (anim->m_sch) {
        TAILQ_REMOVE(&anim->m_sch->m_anims, anim, m_next_for_sch);
    }
    
    anim->m_layer = (void*)env;
    TAILQ_INSERT_TAIL(&env->m_free_anims, anim, m_next_for_layer);
}

void ui_sprite_render_anim_real_free(ui_sprite_render_anim_t anim) {
    ui_sprite_render_env_t env = (void*)anim->m_layer;

    TAILQ_REMOVE(&env->m_free_anims, anim, m_next_for_layer);

    mem_free(env->m_module->m_alloc, anim);
}

ui_sprite_render_anim_t
ui_sprite_render_anim_find(ui_sprite_render_env_t env, uint32_t anim_id) {
    struct ui_sprite_render_anim key;
    key.m_anim_id = anim_id;

    return cpe_hash_table_find(&env->m_anims, &key);
}

uint32_t ui_sprite_render_anim_id(ui_sprite_render_anim_t anim) {
    return anim->m_anim_id;
}

ui_runtime_render_obj_ref_t ui_sprite_render_anim_obj(ui_sprite_render_anim_t anim) {
    return anim->m_render_obj_ref;
}

uint8_t ui_sprite_render_anim_is_runing(ui_sprite_render_anim_t anim) {
    return ui_runtime_render_obj_is_playing(ui_runtime_render_obj_ref_obj(anim->m_render_obj_ref));
}

ui_sprite_render_sch_t ui_sprite_render_anim_sch(ui_sprite_render_anim_t anim) {
    return anim->m_sch;
}

ui_sprite_render_group_t ui_sprite_render_anim_group(ui_sprite_render_anim_t anim) {
    return anim->m_group;
}

ui_sprite_render_layer_t ui_sprite_render_anim_layer(ui_sprite_render_anim_t anim) {
    return anim->m_layer;
}

ui_sprite_render_anim_t ui_sprite_render_anim_find_by_id(ui_sprite_render_sch_t anim_sch, uint32_t anim_id) {
    ui_sprite_render_anim_t anim;

    TAILQ_FOREACH(anim, &anim_sch->m_anims, m_next_for_sch) {
        if (anim->m_anim_id == anim_id) return anim;
    }

    return NULL;
}

ui_sprite_render_anim_t ui_sprite_render_anim_find_by_name(ui_sprite_render_sch_t anim_sch, const char * anim_name) {
    ui_sprite_render_anim_t anim;

    TAILQ_FOREACH(anim, &anim_sch->m_anims, m_next_for_sch) {
        if (strcmp(anim->m_anim_name, anim_name) == 0) return anim;
    }

    return NULL;
}

ui_sprite_render_anim_t ui_sprite_render_anim_find_by_render_obj(ui_sprite_render_sch_t anim_sch, ui_runtime_render_obj_t render_obj) {
    ui_sprite_render_anim_t anim;

    TAILQ_FOREACH(anim, &anim_sch->m_anims, m_next_for_sch) {
        if (ui_runtime_render_obj_ref_obj(anim->m_render_obj_ref) == render_obj) return anim;
    }

    return NULL;
}

ui_transform_t ui_sprite_render_anim_transform(ui_sprite_render_anim_t anim) {
    return &anim->m_transform;
}

void ui_sprite_render_anim_set_transform(ui_sprite_render_anim_t anim, ui_transform_t transform) {
    anim->m_transform = *transform;
    
    if (anim->m_sync_transform) {
        ui_runtime_render_obj_ref_transform_set_to_obj(anim->m_render_obj_ref, transform);
    }
}

int ui_sprite_render_anim_calc_obj_world_transform(ui_sprite_render_anim_t anim, ui_transform_t r) {
    if (anim->m_sync_transform) {
        *r = *ui_runtime_render_obj_transform(ui_runtime_render_obj_ref_obj(anim->m_render_obj_ref));
    }
    else {
        ui_transform_t ref_transform = ui_runtime_render_obj_ref_transform(anim->m_render_obj_ref);

        if (ui_transform_cmp(ref_transform, &UI_TRANSFORM_IDENTITY) != 0) {
            *r = *ref_transform;
        
            if (ui_transform_cmp(&anim->m_transform, &UI_TRANSFORM_IDENTITY) != 0) {
                ui_transform_adj_by_parent(r, &anim->m_transform);
            }
        }
        else {
            *r = anim->m_transform;
        }
    }

    return (cpe_float_cmp(r->m_s.x, 0.0f, UI_FLOAT_PRECISION) == 0
            || cpe_float_cmp(r->m_s.y, 0.0f, UI_FLOAT_PRECISION) == 0
            || cpe_float_cmp(r->m_s.z, 0.0f, UI_FLOAT_PRECISION) == 0)
        ? -1
        : 0;
}

int ui_sprite_render_anim_calc_obj_local_transform(ui_sprite_render_anim_t anim, ui_transform_t r) {
    ui_sprite_entity_t entity = ui_sprite_component_entity(ui_sprite_component_from_data(anim->m_sch));
    ui_sprite_2d_transform_t entity_transform = ui_sprite_2d_transform_find(entity);
    ui_transform entity_trans;

    /*获取entity渲染位置 */
    if (entity_transform == NULL) return -1;
    if (ui_sprite_2d_transform_calc_trans(entity_transform, &entity_trans) != 0) return -1;

    /*动画的世界位置 */
    if (ui_sprite_render_anim_calc_obj_world_transform(anim, r) != 0) return -1;

    /*挑战动画位置到本地转换 */
    /* struct mem_buffer buff; */
    /* mem_buffer_init(&buff, NULL); */
    /* printf("   world anim trans=%s\n", ui_transform_dump_2d(&buff, r)); */
    /* mem_buffer_clear_data(&buff); */
    /* printf("   entity trans origin =%s\n", ui_transform_dump_2d(&buff, &entity_trans)); */
    if (anim->m_group) {
        ui_vector_3 adj_scale;
        uint8_t adj_scale_p = 0;
    
        /*调整本地的处理 */
        if (!anim->m_group->m_accept_flip) {
            adj_scale.x = entity_trans.m_s.x < 0.0f ? entity_trans.m_s.x * -1.0f : entity_trans.m_s.x;
            adj_scale.y = entity_trans.m_s.y < 0.0f ? entity_trans.m_s.y * -1.0f : entity_trans.m_s.y;
            adj_scale.z = entity_trans.m_s.z < 0.0f ? entity_trans.m_s.z * -1.0f : entity_trans.m_s.z;
            adj_scale_p = 1;
        }

        if (!anim->m_group->m_accept_scale) {
            ui_vector_3 p;
            
            if (adj_scale_p == 0) {
                adj_scale = r->m_s;
                adj_scale_p = 1;
            }

            ui_transform_get_pos_3(r, &p);
            p.x *= adj_scale.x;
            p.y *= adj_scale.y;
            p.z *= adj_scale.z;
            ui_transform_set_pos_3(r, &p);
            
            adj_scale.x = 1.0f;
            adj_scale.y = 1.0f;
            adj_scale.z = 1.0f;
        }

        if (adj_scale_p) {
            ui_transform_set_scale(&entity_trans, &adj_scale);
        }

        /*角度直接通过世界旋转处理 */
        if (!anim->m_group->m_accept_rotate) {
            ui_transform_set_quation(&entity_trans, &UI_QUATERNION_IDENTITY);
        }

        /* mem_buffer_clear_data(&buff); */
        /* printf("   entity trans after adj =%s\n", ui_transform_dump_2d(&buff, &entity_trans)); */
    }

    ui_transform_inline_reverse(&entity_trans);
    ui_transform_adj_by_parent(r, &entity_trans);

    /* mem_buffer_clear_data(&buff); */
    /* printf("   local anim trans=%s\n", ui_transform_dump_2d(&buff, r)); */
    /* mem_buffer_clear(&buff); */

    return 0;
}

uint8_t ui_sprite_render_anim_sync_transform(ui_sprite_render_anim_t anim) {
    return anim->m_sync_transform;
}

void ui_sprite_render_anim_set_sync_transform(ui_sprite_render_anim_t anim, uint8_t sync_transform) {
    if (sync_transform) sync_transform = 1;

    if (anim->m_sync_transform == sync_transform) return;

    anim->m_sync_transform = sync_transform;
    if (anim->m_sync_transform) {
        ui_runtime_render_obj_ref_transform_set_to_obj(anim->m_render_obj_ref, &anim->m_transform);
    }
}


uint8_t ui_sprite_render_anim_auto_remove(ui_sprite_render_anim_t anim) {
    return anim->m_auto_remove;
}

void ui_sprite_render_anim_set_auto_remove(ui_sprite_render_anim_t anim, uint8_t auto_remove) {
    if (auto_remove) auto_remove = 1;
    anim->m_auto_remove = auto_remove;
}

float ui_sprite_render_anim_priority(ui_sprite_render_anim_t anim) {
    return anim->m_priority;
}

void ui_sprite_render_anim_set_priority(ui_sprite_render_anim_t anim, float priority) {
    if (anim->m_priority == priority) return;

    anim->m_priority = priority;

    if (anim->m_sch) {
        anim->m_sch->m_is_dirty = 1;
    }

    anim->m_layer->m_is_dirty = 1;
}

static void ui_runtime_render_anim_on_event(void * ctx, ui_runtime_render_obj_t obj, const char * evt) {
    ui_sprite_render_anim_t anim = ctx;
    ui_sprite_render_layer_t layer = anim->m_layer;
    ui_sprite_world_res_t res = ui_sprite_world_res_from_data(layer->m_env);
    ui_sprite_world_t world = ui_sprite_world_res_world(res);

    if (anim->m_sch) {
        ui_sprite_entity_t entity = ui_sprite_component_entity(ui_sprite_component_from_data(anim->m_sch));
        ui_sprite_entity_build_and_send_event(entity, evt, NULL);
    }
    else {
        ui_sprite_world_build_and_send_event(world, evt, NULL);
    }
}

static ui_sprite_render_anim_t ui_sprite_render_layer_anim_next(ui_sprite_render_anim_it_t it) {
    ui_sprite_render_anim_t * data = (ui_sprite_render_anim_t *)(it->m_data);
    ui_sprite_render_anim_t r;

    if (*data == NULL) return NULL;

    r = *data;

    *data = TAILQ_NEXT(r, m_next_for_layer);

    return r;
}

void ui_sprite_render_layer_anims(ui_sprite_render_layer_t layer, ui_sprite_render_anim_it_t anim_it) {
    *(ui_sprite_render_anim_t *)(anim_it->m_data) = TAILQ_FIRST(&layer->m_anims);
    anim_it->next = ui_sprite_render_layer_anim_next;
}

uint32_t ui_sprite_render_anim_hash(ui_sprite_render_anim_t anim) {
    return anim->m_anim_id;
}

int ui_sprite_render_anim_eq(ui_sprite_render_anim_t l, ui_sprite_render_anim_t r) {
    return l->m_anim_id == r->m_anim_id ? 1 : 0;
}
