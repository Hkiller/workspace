#include <assert.h>
#include "cpe/utils/string_utils.h"
#include "render/utils/ui_quaternion.h"
#include "render/utils/ui_transform.h"
#include "render/model/ui_data_mgr.h"
#include "render/model/ui_data_src.h"
#include "plugin/barrage/plugin_barrage_module.h"
#include "plugin/barrage/plugin_barrage_data_barrage.h"
#include "plugin/barrage/plugin_barrage_data_emitter.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite/ui_sprite_component.h"
#include "ui/sprite_2d/ui_sprite_2d_transform.h"
#include "ui/sprite_2d/ui_sprite_2d_part.h"
#include "ui/sprite_2d/ui_sprite_2d_part_binding.h"
#include "ui_sprite_barrage_barrage_i.h"
#include "ui_sprite_barrage_obj_i.h"
#include "ui_sprite_barrage_env_i.h"

static int ui_sprite_barrage_barrage_do_bind(ui_sprite_barrage_barrage_t barrage);

ui_sprite_barrage_barrage_t
ui_sprite_barrage_barrage_create(
    ui_sprite_barrage_obj_t barrage_obj, const char * part_name,
    const char * group, const char * res, plugin_barrage_data_emitter_flip_type_t flip_type)
{
    ui_sprite_barrage_module_t module = barrage_obj->m_module;
	ui_sprite_barrage_barrage_t barrage;
	ui_sprite_entity_t entity = ui_sprite_component_entity(ui_sprite_component_from_data(barrage_obj));
    size_t group_len = strlen(group) + 1;
    size_t part_name_len = strlen(part_name) + 1;
    ui_sprite_2d_transform_t transform = ui_sprite_2d_transform_find(entity);

    if (transform == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): BarrageObj(%s): create barrage: no transform!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), part_name);
        return NULL;
    }

    if (barrage_obj->m_barrage_group == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): BarrageObj(%s): create barrage: barrage group not set!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), part_name);
        return NULL;
    }

    barrage = mem_alloc(module->m_alloc, sizeof(struct ui_sprite_barrage_barrage) + group_len + part_name_len);
    if (barrage == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): BarrageObj(%s): create barrage: alloc fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), barrage->m_part_name);
        return NULL;
    }

    barrage->m_obj = barrage_obj;
    barrage->m_part_name = (const char *)(barrage + 1);
    barrage->m_group = (const char *)(barrage->m_part_name + part_name_len);
    barrage->m_flip_type = flip_type;
    barrage->m_is_enable = 0;
    barrage->m_is_pause = 0;
    barrage->m_barrage = NULL;
    barrage->m_transform = UI_TRANSFORM_IDENTITY;
    barrage->m_accept_flip = 0;
    barrage->m_accept_scale = 0;
    barrage->m_accept_angle = 0;
    barrage->m_angle_adj_rad = 0.0f;
    barrage->m_part = NULL;
    cpe_str_dup(barrage->m_res, sizeof(barrage->m_res), res);
    memcpy((void*)barrage->m_part_name, part_name, part_name_len);
    memcpy((void*)barrage->m_group, group, group_len);

    TAILQ_INSERT_TAIL(&barrage_obj->m_barrages, barrage, m_next_for_obj);

    return barrage;
}

void ui_sprite_barrage_barrage_free(ui_sprite_barrage_barrage_t barrage) {
    ui_sprite_barrage_obj_t obj = barrage->m_obj;
    
    TAILQ_REMOVE(&obj->m_barrages, barrage, m_next_for_obj);

    if (barrage->m_barrage) {
        plugin_barrage_barrage_free(barrage->m_barrage);
        barrage->m_barrage = NULL;
    }

    mem_free(obj->m_module->m_alloc, barrage);
}

int ui_sprite_barrage_barrage_load(ui_sprite_barrage_barrage_t barrage) {
    ui_sprite_barrage_obj_t barrage_obj = barrage->m_obj;
    ui_sprite_barrage_module_t module = barrage_obj->m_module;
	ui_sprite_entity_t entity = ui_sprite_component_entity(ui_sprite_component_from_data(barrage_obj));
    ui_data_src_t barrage_src;
    plugin_barrage_data_barrage_t data_barrage;
    struct plugin_barrage_data_emitter_it data_emitter_it;
    plugin_barrage_data_emitter_t data_emitter;
    
    if (barrage->m_barrage != NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): BarrageObj(%s): load barrage: barrage already loaded!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), barrage->m_group);
        return -1;
    }

    if (ui_sprite_barrage_barrage_do_bind(barrage) != 0) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): BarrageObj(%s): load barrage: create part fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), barrage->m_group);
        return -1;
    }

    barrage_src =
        ui_data_src_child_find_by_path(
            ui_data_mgr_src_root(plugin_barrage_module_data_mgr(module->m_barrage_module)),
            barrage->m_res, ui_data_src_type_barrage);
    if (barrage_src == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): BarrageObj(%s): create barrage: barrage %s not exist!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), barrage->m_group, barrage->m_res);
        barrage->m_part = NULL;
        return -1;
    }
    
    if (!ui_data_src_is_loaded(barrage_src)) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): BarrageObj(%s): create barrage: barrage %s not loaded!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), barrage->m_group, barrage->m_res);
        barrage->m_part = NULL;
        return -1;
    }
    data_barrage = ui_data_src_product(barrage_src);

    barrage->m_barrage = plugin_barrage_barrage_create(barrage_obj->m_barrage_group);
    if (barrage->m_barrage == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): BarrageObj(%s): create barrage: create barrage fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), barrage->m_group);
        barrage->m_part = NULL;
        return -1;
    }

    plugin_barrage_data_barrage_emitters(&data_emitter_it, data_barrage);
    while((data_emitter = plugin_barrage_data_emitter_it_next(&data_emitter_it))) {
        plugin_barrage_emitter_t emitter = plugin_barrage_emitter_create(barrage->m_barrage, data_emitter, barrage->m_flip_type);
        if (emitter == NULL) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): BarrageObj(%s): create barrage: create emitter fail!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), barrage->m_group);
            plugin_barrage_barrage_free(barrage->m_barrage);
            barrage->m_barrage = NULL;
            barrage->m_part = NULL;
            return -1;
        }
    }
    
    if (barrage_obj->m_target_fun) {
        plugin_barrage_barrage_set_target_fun(barrage->m_barrage, barrage_obj->m_target_fun, barrage_obj->m_target_fun_ctx);
    }
    
    plugin_barrage_barrage_set_collision_info(
        barrage->m_barrage,
        barrage_obj->m_collision_category,
        barrage_obj->m_collision_mask,
        barrage_obj->m_collision_group);
    
    plugin_barrage_barrage_set_show_dead_anim_mask(
        barrage->m_barrage,
        barrage_obj->m_collision_show_dead_anim_mask);
    
    return 0;
}

void ui_sprite_barrage_barrage_unload(ui_sprite_barrage_barrage_t barrage) {
    if (barrage->m_barrage) {
        plugin_barrage_barrage_free(barrage->m_barrage);
        barrage->m_barrage = NULL;
    }
}

void ui_sprite_barrage_barrage_free_group(ui_sprite_barrage_obj_t barrage_obj, const char * group) {
    ui_sprite_barrage_barrage_t barrage, next;

    for(barrage = TAILQ_FIRST(&barrage_obj->m_barrages); barrage; barrage = next) {
        next = TAILQ_NEXT(barrage, m_next_for_obj);

        if (strcmp(barrage->m_group, group) == 0) {
            ui_sprite_barrage_barrage_free(barrage);
        }
    }
}

void ui_sprite_barrage_barrage_free_all(ui_sprite_barrage_obj_t obj) {
    while(!TAILQ_EMPTY(&obj->m_barrages)) {
        ui_sprite_barrage_barrage_free(TAILQ_FIRST(&obj->m_barrages));
    }
}

plugin_barrage_barrage_t ui_sprite_barrage_barrage_barrage(ui_sprite_barrage_barrage_t barrage) {
    return barrage->m_barrage;
}

void ui_sprite_barrage_barrage_update_pos(ui_sprite_barrage_barrage_t barrage) {
    ui_transform trans;
    ui_transform entity_trans;
    
    assert(barrage->m_barrage);
    assert(barrage->m_part);

    trans = *ui_sprite_2d_part_trans(barrage->m_part);

    if (!barrage->m_accept_angle) {
        if (barrage->m_angle_adj_rad != 0.0f) {
            ui_quaternion q;
            ui_quaternion_set_z_radians(&q, barrage->m_angle_adj_rad);
            ui_transform_set_quation(&trans, &q);
        }
        else {
            ui_transform_set_quation(&trans, &UI_QUATERNION_IDENTITY);
        }
    }
    else {
        if (barrage->m_angle_adj_rad) {
            float angle_rad = ui_transform_calc_angle_z_rad(&trans);
            ui_quaternion q;

            angle_rad +=
                trans.m_s.x * trans.m_s.y < 0.0f
                ? - barrage->m_angle_adj_rad
                : + barrage->m_angle_adj_rad;
            
            ui_quaternion_set_z_radians(&q, angle_rad);
            ui_transform_set_quation(&trans, &q);
        }
    }
    
    if (!barrage->m_accept_scale) {
        if (barrage->m_accept_flip) {
            ui_vector_3 s = trans.m_s;
            s.x = s.x > 0.0f ? 1.0f : -1.0f;
            s.y = s.y > 0.0f ? 1.0f : -1.0f;
            s.z = s.z > 0.0f ? 1.0f : -1.0f;
        }
        else {
            ui_transform_set_scale(&trans, &UI_VECTOR_3_IDENTITY);  
        }
    }
    else {
        if (!barrage->m_accept_flip) {
            ui_vector_3 s = trans.m_s;
            if (s.x < 0.0f) s.x = - s.x;
            if (s.y < 0.0f) s.y = - s.y;
            if (s.z < 0.0f) s.z = - s.z;
        }
    }

    
    /*根据entity的位置调整 */
    if (ui_sprite_2d_transform_calc_trans(ui_sprite_2d_part_transform(barrage->m_part), &entity_trans) != 0) return;
    
    ui_transform_adj_by_parent(&trans, &entity_trans);
        
    plugin_barrage_barrage_set_transform(barrage->m_barrage, &trans);
}

void ui_sprite_barrage_barrage_set_transform(ui_sprite_barrage_barrage_t barrage, ui_transform_t transform) {
    barrage->m_transform = *transform;
}

void ui_sprite_barrage_barrage_sync_state(ui_sprite_barrage_barrage_t barrage) {
    if (barrage->m_is_enable && !barrage->m_is_pause) {
        if (!plugin_barrage_barrage_is_enable(barrage->m_barrage)) {
            plugin_barrage_barrage_enable(barrage->m_barrage, barrage->m_loop_count);
        }
    }
    else {
        if (plugin_barrage_barrage_is_enable(barrage->m_barrage)) {
            plugin_barrage_barrage_disable(barrage->m_barrage, 0);
        }
    }
}

static void ui_sprite_barrage_barrage_on_update(ui_sprite_2d_part_t part, void * ctx) {
    if (ui_sprite_2d_part_trans_updated(part)) {
        ui_sprite_barrage_barrage_update_pos(ctx);
    }
}

int ui_sprite_barrage_barrage_do_bind(ui_sprite_barrage_barrage_t barrage) {
    ui_sprite_barrage_module_t module = barrage->m_obj->m_module;
    ui_sprite_component_t component = ui_sprite_component_from_data(barrage->m_obj);
    ui_sprite_entity_t entity = ui_sprite_component_entity(component);
    ui_sprite_2d_transform_t transform = ui_sprite_2d_transform_find(entity);
 
    assert(barrage->m_part == NULL);

    barrage->m_part = ui_sprite_2d_part_find(transform, barrage->m_part_name);
    if (barrage->m_part == NULL) {
        barrage->m_part  = ui_sprite_2d_part_create(transform, barrage->m_part_name);
        if (barrage->m_part  == NULL) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): barrage %s: create part fail!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), barrage->m_part_name);
            return -1;
        }

        ui_sprite_2d_part_set_trans(barrage->m_part, &barrage->m_transform);
        ui_sprite_2d_part_dispatch_event(barrage->m_part);
    }
    
    if (ui_sprite_2d_part_binding_create(barrage->m_part , barrage, ui_sprite_barrage_barrage_on_update) == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): barrage %s: create bidning fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), barrage->m_part_name);
        return -1;
    }

    return 0;
}
