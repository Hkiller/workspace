#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/math_ex.h"
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/utils/string_utils.h"
#include "gd/app/app_context.h"
#include "render/model/ui_data_src.h"
#include "render/model/ui_data_mgr.h"
#include "plugin/chipmunk/plugin_chipmunk_env.h"
#include "plugin/barrage/plugin_barrage_group.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite/ui_sprite_entity_calc.h"
#include "ui/sprite/ui_sprite_component.h"
#include "ui/sprite/ui_sprite_component_meta.h"
#include "ui/sprite_chipmunk/ui_sprite_chipmunk_env.h"
#include "protocol/ui/sprite_2d/ui_sprite_2d_data.h"
#include "ui/sprite_2d/ui_sprite_2d_transform.h"
#include "ui/sprite_2d/ui_sprite_2d_part.h"
#include "ui_sprite_barrage_obj_i.h"
#include "ui_sprite_barrage_env_i.h"
#include "ui_sprite_barrage_barrage_i.h"

#ifdef __cplusplus
extern "C" {
#endif

ui_sprite_barrage_obj_t ui_sprite_barrage_obj_create(ui_sprite_entity_t entity) {
    ui_sprite_component_t component = ui_sprite_component_create(entity, UI_SPRITE_BARRAGE_OBJ_NAME);
    return component ? (ui_sprite_barrage_obj_t)ui_sprite_component_data(component) : NULL;
};

ui_sprite_barrage_obj_t ui_sprite_barrage_obj_find(ui_sprite_entity_t entity) {
    ui_sprite_component_t component = ui_sprite_component_find(entity, UI_SPRITE_BARRAGE_OBJ_NAME);
    return component ? (ui_sprite_barrage_obj_t)ui_sprite_component_data(component) : NULL;
};

void ui_sprite_barrage_obj_free(ui_sprite_barrage_obj_t barrage_obj) {
    ui_sprite_component_t component = ui_sprite_component_from_data(barrage_obj);
    if (component) {
        ui_sprite_component_free(component);
    }
}

void ui_sprite_barrage_obj_set_target_fun(ui_sprite_barrage_obj_t barrage_obj, plugin_barrage_target_fun_t fun, void * ctx) {
    ui_sprite_barrage_barrage_t barrage;

    barrage_obj->m_target_fun = fun;
    barrage_obj->m_target_fun_ctx = ctx;

    TAILQ_FOREACH(barrage, &barrage_obj->m_barrages, m_next_for_obj) {
        plugin_barrage_barrage_set_target_fun(barrage->m_barrage, ctx, fun);
    }
}
    
int ui_sprite_barrage_obj_crate_barrages(ui_sprite_barrage_obj_t barrage_obj, cfg_t cfg) {
    ui_sprite_barrage_module_t module = barrage_obj->m_module;
    ui_sprite_entity_t entity = ui_sprite_component_entity(ui_sprite_component_from_data(barrage_obj));
    struct cfg_it barrage_cfgs;
    cfg_t barrage_cfg;
    ui_transform transform = UI_TRANSFORM_IDENTITY;
	ui_vector_2 pos;
    
	if(cfg_type(cfg) == CPE_CFG_TYPE_STRING) {
        cfg_t root_cfg = cfg;
        while(cfg_parent(root_cfg)) root_cfg = cfg_parent(root_cfg);
        cfg_it_init(&barrage_cfgs, cfg_find_cfg(root_cfg, cfg_as_string(cfg, NULL)));
    }
    else {
        cfg_it_init(&barrage_cfgs, cfg);
    }

    while((barrage_cfg = cfg_it_next(&barrage_cfgs))) {
        const char * part_name = cfg_get_string(barrage_cfg, "binding-part", "");
        const char * group = cfg_get_string(barrage_cfg, "group", NULL);
        const char * use = cfg_get_string(barrage_cfg, "use", NULL);
        const char * str_flip_type = cfg_get_string(barrage_cfg, "flip", NULL);
        plugin_barrage_data_emitter_flip_type_t flip_type = plugin_barrage_data_emitter_flip_type_none;
        ui_sprite_barrage_barrage_t barrage;
        cfg_t angle_adj;
        
        if (group == NULL) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): create barrages: group not configured!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
            return -1;
        }

		if (use == NULL) {
			CPE_ERROR(
				module->m_em, "entity %d(%s): create barrages: use not configured!",
				ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
			return -1;
		}

        if (str_flip_type) {
            if (strcmp(str_flip_type, "flip-none") == 0) {
                flip_type = plugin_barrage_data_emitter_flip_type_none;
            }
            else if (strcmp(str_flip_type, "flip-x") == 0) {
                flip_type = plugin_barrage_data_emitter_flip_type_x;
            }
            else if (strcmp(str_flip_type, "flip-y") == 0) {
                flip_type = plugin_barrage_data_emitter_flip_type_y;
            }
            else if (strcmp(str_flip_type, "flip-xy") == 0) {
                flip_type = plugin_barrage_data_emitter_flip_type_xy;
            }
            else {
                CPE_ERROR(
                    module->m_em, "entity %d(%s): create barrages: flip type %s unknown!",
                    ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), str_flip_type);
                return -1;
            }
        }

        barrage = ui_sprite_barrage_barrage_create(barrage_obj, part_name, group, use, flip_type);
        if (barrage == NULL) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): create barrages: create barrage %s(use=%s) fail!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), group, use);
            return -1;
        }

        pos.x = cfg_get_float(barrage_cfg, "pos.x", 0.0f);
		pos.y = cfg_get_float(barrage_cfg, "pos.y", 0.0f);
        ui_transform_set_pos_2(&transform, &pos);
        
		ui_sprite_barrage_barrage_set_transform(barrage, &transform);

        barrage->m_accept_flip = cfg_get_uint8(barrage_cfg, "accept-flip", barrage->m_accept_flip);
        barrage->m_accept_scale = cfg_get_uint8(barrage_cfg, "accept-scale", barrage->m_accept_scale);
        barrage->m_accept_angle = cfg_get_uint8(barrage_cfg, "accept-angle", barrage->m_accept_angle);

        if ((angle_adj = cfg_find_cfg(barrage_cfg, "angle-adj"))) {
            float v;
            if (cfg_try_as_float(angle_adj, &v) == 0) {
                barrage->m_angle_adj_rad = cpe_math_angle_to_radians(v);
            }
        }
    }

    return 0;
}

static int ui_sprite_barrage_obj_do_init(ui_sprite_barrage_module_t module, ui_sprite_barrage_obj_t obj, ui_sprite_entity_t entity) {
    bzero(obj, sizeof(*obj));

    obj->m_module = module;
    obj->m_barrage_group = NULL;
    obj->m_speed_adj[0] = 0;
    obj->m_emitter_adj[0] = 0;
    obj->m_collision_category = 0;
    obj->m_collision_mask = 0;
    obj->m_collision_group = 0;
    obj->m_target_fun = NULL;
    obj->m_dft_collision_event = NULL;
    
    TAILQ_INIT(&obj->m_barrages);

    return 0;
}

static int ui_sprite_barrage_obj_init(ui_sprite_component_t component, void * ctx) {
    ui_sprite_barrage_module_t module = (ui_sprite_barrage_module_t)ctx;
    ui_sprite_entity_t entity = ui_sprite_component_entity(component);
    ui_sprite_barrage_obj_t barrage_obj = (ui_sprite_barrage_obj_t)ui_sprite_component_data(component);

    if (ui_sprite_barrage_obj_do_init(module, barrage_obj, entity) != 0) return -1;

    return 0;
}

static void ui_sprite_barrage_obj_do_fini(ui_sprite_barrage_module_t module, ui_sprite_barrage_obj_t barrage_obj) {
    ui_sprite_barrage_barrage_free_all(barrage_obj);
    assert(TAILQ_EMPTY(&barrage_obj->m_barrages));

    if (barrage_obj->m_dft_collision_event) {
        mem_free(module->m_alloc, barrage_obj->m_dft_collision_event);
        barrage_obj->m_dft_collision_event = NULL;
    }
}

static void ui_sprite_barrage_obj_fini(ui_sprite_component_t component, void * ctx) {
    ui_sprite_barrage_obj_t barrage_obj = (ui_sprite_barrage_obj_t)ui_sprite_component_data(component);
    ui_sprite_barrage_obj_do_fini((ui_sprite_barrage_module_t)ctx, barrage_obj);
}

static int ui_sprite_barrage_obj_copy(ui_sprite_component_t to, ui_sprite_component_t from, void * ctx) {
    ui_sprite_barrage_module_t module = (ui_sprite_barrage_module_t)ctx;
    ui_sprite_entity_t entity = ui_sprite_component_entity(to);
    ui_sprite_barrage_obj_t from_barrage_obj = (ui_sprite_barrage_obj_t)ui_sprite_component_data(from);
    ui_sprite_barrage_obj_t to_barrage_obj = (ui_sprite_barrage_obj_t)ui_sprite_component_data(to);
    ui_sprite_barrage_barrage_t from_barrage;

    if (ui_sprite_barrage_obj_do_init(module, to_barrage_obj, entity) != 0) return -1;

    cpe_str_dup(to_barrage_obj->m_speed_adj, sizeof(to_barrage_obj->m_speed_adj), from_barrage_obj->m_speed_adj);
    cpe_str_dup(to_barrage_obj->m_emitter_adj, sizeof(to_barrage_obj->m_emitter_adj), from_barrage_obj->m_emitter_adj);
    to_barrage_obj->m_barrage_group = from_barrage_obj->m_barrage_group;
    to_barrage_obj->m_collision_category = from_barrage_obj->m_collision_category;
    to_barrage_obj->m_collision_mask = from_barrage_obj->m_collision_mask;
    to_barrage_obj->m_collision_show_dead_anim_mask = from_barrage_obj->m_collision_show_dead_anim_mask;
    to_barrage_obj->m_collision_group = from_barrage_obj->m_collision_group;
    if (from_barrage_obj->m_dft_collision_event) to_barrage_obj->m_dft_collision_event = cpe_str_mem_dup(module->m_alloc, from_barrage_obj->m_dft_collision_event);
    
    TAILQ_FOREACH(from_barrage, &from_barrage_obj->m_barrages, m_next_for_obj) {
        ui_sprite_barrage_barrage_t to_barrage;

        to_barrage = ui_sprite_barrage_barrage_create(
            to_barrage_obj, from_barrage->m_part_name, from_barrage->m_group, from_barrage->m_res, from_barrage->m_flip_type);
        if (to_barrage == NULL) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): create barrages: create barrage %s(use=%s) fail!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity),
                from_barrage->m_group, from_barrage->m_res);
            ui_sprite_barrage_obj_do_fini(module, to_barrage_obj);
            return -1;
        }

        to_barrage->m_transform = from_barrage->m_transform;
        to_barrage->m_accept_flip = from_barrage->m_accept_flip;
        to_barrage->m_accept_scale = from_barrage->m_accept_scale;
        to_barrage->m_accept_angle = from_barrage->m_accept_angle;
        to_barrage->m_angle_adj_rad = from_barrage->m_angle_adj_rad;
    }
    
    return 0;
}

static void ui_sprite_barrage_obj_exit(ui_sprite_component_t component, void * ctx) {
    ui_sprite_barrage_obj_t barrage_obj = (ui_sprite_barrage_obj_t)ui_sprite_component_data(component);
    ui_sprite_barrage_barrage_t barrage;

    ui_sprite_barrage_obj_disable_barrages(barrage_obj, NULL, 0);

    TAILQ_FOREACH(barrage, &barrage_obj->m_barrages, m_next_for_obj) {
        ui_sprite_barrage_barrage_unload(barrage);
    }
}

static void ui_sprite_barrage_obj_on_transform_update(void * ctx) {
    ui_sprite_barrage_barrage_t barrage;
    ui_sprite_barrage_obj_t barrage_obj = (ui_sprite_barrage_obj_t)ctx;

    TAILQ_FOREACH(barrage, &barrage_obj->m_barrages, m_next_for_obj) {
        ui_sprite_barrage_barrage_update_pos(barrage);
    }
}

static void ui_sprite_barrage_obj_on_speed_adj_update(void * ctx) {
    ui_sprite_barrage_barrage_t barrage;
    ui_sprite_barrage_obj_t barrage_obj = (ui_sprite_barrage_obj_t)ctx;
    ui_sprite_barrage_module_t module = barrage_obj->m_module;
    ui_sprite_entity_t entity = ui_sprite_component_entity(ui_sprite_component_from_data(barrage_obj));
    float speed_adj;

    if (ui_sprite_entity_try_calc_float(
            &speed_adj, barrage_obj->m_speed_adj, entity, NULL, barrage_obj->m_module->m_em)
        != 0)
    {
        CPE_ERROR(
            module->m_em, "entity %d(%s): on speed adj update: calc value from '%s' fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), barrage_obj->m_speed_adj);
        return;
    }

    if (speed_adj <= 0.0f) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): on speed adj update: speed adj %f error!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), speed_adj);
        return;
    }

    TAILQ_FOREACH(barrage, &barrage_obj->m_barrages, m_next_for_obj) {
        plugin_barrage_barrage_set_speed_adj(barrage->m_barrage, speed_adj);
    }
}

static void ui_sprite_barrage_obj_on_emitter_adj_update(void * ctx) {
    ui_sprite_barrage_barrage_t barrage;
    ui_sprite_barrage_obj_t barrage_obj = (ui_sprite_barrage_obj_t)ctx;
    ui_sprite_barrage_module_t module = barrage_obj->m_module;
    ui_sprite_entity_t entity = ui_sprite_component_entity(ui_sprite_component_from_data(barrage_obj));
    float emitter_adj;

    if (ui_sprite_entity_try_calc_float(
            &emitter_adj, barrage_obj->m_emitter_adj, entity, NULL, barrage_obj->m_module->m_em)
        != 0)
    {
        CPE_ERROR(
            module->m_em, "entity %d(%s): on barrage adj update: calc value from '%s' fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), barrage_obj->m_emitter_adj);
        return;
    }

    if (emitter_adj <= 0.0f) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): on barrage adj update: barrage adj %f error!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), emitter_adj);
        return;
    }

    TAILQ_FOREACH(barrage, &barrage_obj->m_barrages, m_next_for_obj) {
        plugin_barrage_barrage_set_emitter_adj(barrage->m_barrage, emitter_adj);
    }
}

static int ui_sprite_barrage_obj_enter(ui_sprite_component_t component, void * ctx) {
    ui_sprite_barrage_module_t module = (ui_sprite_barrage_module_t)ctx;
    ui_sprite_barrage_obj_t barrage_obj = (ui_sprite_barrage_obj_t)ui_sprite_component_data(component);
    ui_sprite_entity_t entity = ui_sprite_component_entity(component);
    ui_sprite_2d_transform_t transform;
    ui_sprite_barrage_barrage_t barrage;
    
    if (barrage_obj->m_barrage_group == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): barrage obj enter: barrage group not set!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }

    TAILQ_FOREACH(barrage, &barrage_obj->m_barrages, m_next_for_obj) {
        if (ui_sprite_barrage_barrage_load(barrage) != 0) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): barrage obj load: load barrage fail!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
            goto ENTER_FAIL;
        }
    }

    
    if ((transform = ui_sprite_2d_transform_find(entity))) {
        ui_sprite_barrage_obj_on_transform_update(barrage_obj);

        if (ui_sprite_component_add_attr_monitor(
                component, "transform.pos,transform.angle",
                ui_sprite_barrage_obj_on_transform_update, barrage_obj)
            == NULL)
        {
            CPE_ERROR(
                module->m_em, "entity %d(%s): barrage obj enter: add attr montor fail!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
            goto ENTER_FAIL;
        }
    }

    if (barrage_obj->m_speed_adj[0]) {
        ui_sprite_barrage_obj_on_speed_adj_update(barrage_obj);

        if (barrage_obj->m_speed_adj[0] == '@') {
            if (ui_sprite_component_add_attr_monitor(
                    component, barrage_obj->m_speed_adj + 1,
                    ui_sprite_barrage_obj_on_speed_adj_update, barrage_obj)
                == NULL)
            {
                CPE_ERROR(
                    module->m_em, "entity %d(%s): barrage obj enter: add attr %s montor for speed-adj fail!",
                    ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), barrage_obj->m_speed_adj + 1);
                goto ENTER_FAIL;
            }
        }
    }

    if (barrage_obj->m_emitter_adj[0]) {
        ui_sprite_barrage_obj_on_emitter_adj_update(barrage_obj);

        if (barrage_obj->m_emitter_adj[0] == '@') {
            if (ui_sprite_component_add_attr_monitor(
                    component, barrage_obj->m_emitter_adj + 1,
                    ui_sprite_barrage_obj_on_emitter_adj_update, barrage_obj)
                == NULL)
            {
                CPE_ERROR(
                    module->m_em, "entity %d(%s): barrage obj enter: add attr %s montor for barrage-adj fail!",
                    ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), barrage_obj->m_emitter_adj + 1);
                goto ENTER_FAIL;
            }
        }
    }
    
    return 0;

ENTER_FAIL:
    TAILQ_FOREACH(barrage, &barrage_obj->m_barrages, m_next_for_obj) {
        ui_sprite_barrage_barrage_unload(barrage);
    }

    return -1;
}

static int ui_sprite_barrage_obj_load(void * ctx, ui_sprite_component_t comp, cfg_t cfg) {
    ui_sprite_barrage_module_t module = (ui_sprite_barrage_module_t)ctx;
    ui_sprite_entity_t entity = ui_sprite_component_entity(comp);
    ui_sprite_world_t world = ui_sprite_entity_world(entity);
    ui_sprite_chipmunk_env_t sprite_chipmunk_env = ui_sprite_chipmunk_env_find(world);
    plugin_chipmunk_env_t chipmunk_env;
    ui_sprite_barrage_obj_t barrage_obj = (ui_sprite_barrage_obj_t)ui_sprite_component_data(comp);
    ui_sprite_barrage_env_t sprite_barrage_env = ui_sprite_barrage_env_find(world);
    cfg_t barrage_cfg;
    const char * value;

    if (sprite_barrage_env == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): barrage obj create: no barrage env!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }

    if (sprite_chipmunk_env == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): barrage obj create: no chipmunk env!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }

    chipmunk_env = ui_sprite_chipmunk_env_env(sprite_chipmunk_env);
    assert(chipmunk_env);

    if ((value = cfg_get_string(cfg, "barrage-group", NULL))) {
        barrage_obj->m_barrage_group = plugin_barrage_group_find(sprite_barrage_env->m_env, value);
        if (barrage_obj->m_barrage_group == NULL) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): barrage obj create: barrage group %s not exist!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), value);
            return -1;
            
        }
    }

    if ((value = cfg_get_string(cfg, "speed-adj", NULL))) {
        cpe_str_dup(barrage_obj->m_speed_adj, sizeof(barrage_obj->m_speed_adj), value);
    }

    if ((value = cfg_get_string(cfg, "barrage-adj", NULL))) {
        cpe_str_dup(barrage_obj->m_emitter_adj, sizeof(barrage_obj->m_emitter_adj), value);
    }

    if ((value = cfg_get_string(cfg, "collision.category", NULL))) {
        uint32_t v;
        if (plugin_chipmunk_env_masks(chipmunk_env, &v, value) != 0) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): barrage obj create: load category from '%s' fail!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), value);
            return -1;
        }
        
        barrage_obj->m_collision_category |= v;
    }

    if ((value = cfg_get_string(cfg, "collision.mask", NULL))) {    
        uint32_t v;

        if (plugin_chipmunk_env_masks(chipmunk_env, &v, value) != 0) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): barrage obj create: load mask from '%s' fail!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), value);
            return -1;
        }

        barrage_obj->m_collision_mask |= v;
    }

    if ((value = cfg_get_string(cfg, "collision.event", NULL))) {
        barrage_obj->m_dft_collision_event = cpe_str_mem_dup(module->m_alloc, value);
    }
    
    if ((value = cfg_get_string(cfg, "collision.show-dead-anim-mask", NULL))) {
        uint32_t v;

        if (plugin_chipmunk_env_masks(chipmunk_env, &v, value) != 0) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): barrage obj create: load show-dead-anim-mask from '%s' fail!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), value);
            return -1;
        }

        barrage_obj->m_collision_show_dead_anim_mask |= v;
    }
    
    barrage_obj->m_collision_group = cfg_get_uint32(cfg, "collision.group", barrage_obj->m_collision_group);
    
    if ((barrage_cfg = cfg_find_cfg(cfg, "barrages"))) {
        if (ui_sprite_barrage_obj_crate_barrages(barrage_obj, barrage_cfg) != 0) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): barrage obj create: load barrages fail!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
            return -1;
        }
    }

    return 0;
}

void ui_sprite_barrage_obj_set_mask(ui_sprite_barrage_obj_t barrage_obj, const char * barrage_perfix, uint32_t mask) {
    ui_sprite_barrage_barrage_t barrage;

    barrage_obj->m_collision_mask = mask;
    if(barrage_perfix && barrage_perfix[0] != 0)
    {
        TAILQ_FOREACH(barrage, &barrage_obj->m_barrages, m_next_for_obj) {
            if (cpe_str_start_with(barrage->m_group, barrage_perfix)) {
                plugin_barrage_barrage_set_collision_info(barrage->m_barrage, barrage_obj->m_collision_category,
                    barrage_obj->m_collision_mask,
                    barrage_obj->m_collision_group);
            }
        }
    }
    else
    {
        TAILQ_FOREACH(barrage, &barrage_obj->m_barrages, m_next_for_obj) {
            plugin_barrage_barrage_set_collision_info(barrage->m_barrage, barrage_obj->m_collision_category,
                barrage_obj->m_collision_mask,
                barrage_obj->m_collision_group);
        }
    }
}

uint32_t ui_sprite_barrage_obj_mask(ui_sprite_barrage_obj_t barrage_obj) {
    return barrage_obj->m_collision_mask;
}


void ui_sprite_barrage_obj_pause_barrages(ui_sprite_barrage_obj_t barrage_obj, const char * group_name) {
    ui_sprite_barrage_barrage_t barrage;

    if (group_name == NULL) {
        TAILQ_FOREACH(barrage, &barrage_obj->m_barrages, m_next_for_obj) {
            barrage->m_is_pause = 1;
            barrage->m_is_enable = plugin_barrage_barrage_is_enable(barrage->m_barrage);
            barrage->m_loop_count = plugin_barrage_barrage_loop_count(barrage->m_barrage);
            ui_sprite_barrage_barrage_sync_state(barrage);
        }
    }
    else {
        TAILQ_FOREACH(barrage, &barrage_obj->m_barrages, m_next_for_obj) {
            if (strcmp(barrage->m_group, group_name) == 0) {
                barrage->m_is_pause = 1;
                barrage->m_is_enable = plugin_barrage_barrage_is_enable(barrage->m_barrage);
                barrage->m_loop_count = plugin_barrage_barrage_loop_count(barrage->m_barrage);
                ui_sprite_barrage_barrage_sync_state(barrage);
            }
        }
    }
}
    
void ui_sprite_barrage_obj_resume_barrages(ui_sprite_barrage_obj_t barrage_obj, const char * group_name) {
    ui_sprite_barrage_barrage_t barrage;

    if (group_name == NULL) {
        TAILQ_FOREACH(barrage, &barrage_obj->m_barrages, m_next_for_obj) {
            barrage->m_is_pause = 0;
            ui_sprite_barrage_barrage_sync_state(barrage);
        }
    }
    else {
        TAILQ_FOREACH(barrage, &barrage_obj->m_barrages, m_next_for_obj) {
            if (strcmp(barrage->m_group, group_name) == 0) {
                barrage->m_is_pause = 0;
                ui_sprite_barrage_barrage_sync_state(barrage);
            }
        }
    }
}

uint8_t ui_sprite_barrage_obj_sync_barrages(ui_sprite_barrage_obj_t barrage_obj, const char * group_name) {
    uint8_t is_runing = 0;
    ui_sprite_barrage_barrage_t barrage;

    if (group_name == NULL) {
        TAILQ_FOREACH(barrage, &barrage_obj->m_barrages, m_next_for_obj) {
            barrage->m_is_enable = plugin_barrage_barrage_is_enable(barrage->m_barrage);
            barrage->m_loop_count = plugin_barrage_barrage_loop_count(barrage->m_barrage);
            if (barrage->m_is_enable) is_runing = 1;
        }
    }
    else {
        TAILQ_FOREACH(barrage, &barrage_obj->m_barrages, m_next_for_obj) {
            if (strcmp(barrage->m_group, group_name) == 0) {
                barrage->m_is_enable = plugin_barrage_barrage_is_enable(barrage->m_barrage);
                barrage->m_loop_count = plugin_barrage_barrage_loop_count(barrage->m_barrage);
                if (barrage->m_is_enable) is_runing = 1;
            }
        }
    }

    return is_runing;
}
    
void ui_sprite_barrage_obj_enable_barrages(
    ui_sprite_barrage_obj_t barrage_obj, const char * group_name, ui_sprite_event_t collision_event, uint32_t loop_count)
{
    ui_sprite_barrage_barrage_t barrage;
    struct dr_data carry_data;

    carry_data.m_meta = collision_event->meta;
    carry_data.m_data = (void*)collision_event->data;
    carry_data.m_size = collision_event->size;

    if (group_name == NULL) {
        TAILQ_FOREACH(barrage, &barrage_obj->m_barrages, m_next_for_obj) {
            plugin_barrage_barrage_set_carray_data(barrage->m_barrage, &carry_data);
            barrage->m_loop_count = loop_count;
            barrage->m_is_enable = 1;
            ui_sprite_barrage_barrage_sync_state(barrage);
        }
    }
    else {
        TAILQ_FOREACH(barrage, &barrage_obj->m_barrages, m_next_for_obj) {
            if (strcmp(barrage->m_group, group_name) == 0) {
                plugin_barrage_barrage_set_carray_data(barrage->m_barrage, &carry_data);
                barrage->m_loop_count = loop_count;
                barrage->m_is_enable = 1;
                ui_sprite_barrage_barrage_sync_state(barrage);
            }
        }
    }
}

void ui_sprite_barrage_obj_disable_barrages(ui_sprite_barrage_obj_t barrage_obj, const char * group_name, uint8_t destory_bullets) {
    ui_sprite_barrage_barrage_t barrage;

    if (group_name == NULL) {
        TAILQ_FOREACH(barrage, &barrage_obj->m_barrages, m_next_for_obj) {
            plugin_barrage_barrage_disable(barrage->m_barrage, destory_bullets);
            barrage->m_is_enable = 0;
        }
    }
    else {
        TAILQ_FOREACH(barrage, &barrage_obj->m_barrages, m_next_for_obj) {
            if (strcmp(barrage->m_group, group_name) == 0) {
                plugin_barrage_barrage_disable(barrage->m_barrage, destory_bullets);
                barrage->m_is_enable = 0;
            }
        }
    }
}

void ui_sprite_barrage_obj_clear_bullets(ui_sprite_barrage_obj_t barrage_obj, const char * group_name) {
    ui_sprite_barrage_barrage_t barrage;

    if (group_name == NULL) {
        TAILQ_FOREACH(barrage, &barrage_obj->m_barrages, m_next_for_obj) {
            plugin_barrage_barrage_clear_bullets(barrage->m_barrage);
        }
    }
    else {
        TAILQ_FOREACH(barrage, &barrage_obj->m_barrages, m_next_for_obj) {
            if (strcmp(barrage->m_group, group_name) == 0) {
                plugin_barrage_barrage_clear_bullets(barrage->m_barrage);
            }
        }
    }
}

uint8_t ui_sprite_barrage_obj_is_barrages_enable(ui_sprite_barrage_obj_t barrage_obj, const char * group_name) {
    ui_sprite_barrage_barrage_t barrage;

    if (group_name == NULL) {
        TAILQ_FOREACH(barrage, &barrage_obj->m_barrages, m_next_for_obj) {
            if (plugin_barrage_barrage_is_enable(barrage->m_barrage)) return 1;
        }
    }
    else {
        TAILQ_FOREACH(barrage, &barrage_obj->m_barrages, m_next_for_obj) {
            if (strcmp(barrage->m_group, group_name) == 0) {
                if (plugin_barrage_barrage_is_enable(barrage->m_barrage)) return 1;
            }
        }
    }

    return 0;
}

int ui_sprite_barrage_obj_regist(ui_sprite_barrage_module_t module) {
    ui_sprite_component_meta_t meta;

    meta = ui_sprite_component_meta_create(
        module->m_repo,
        UI_SPRITE_BARRAGE_OBJ_NAME, sizeof(struct ui_sprite_barrage_obj));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: %s register: meta create fail",
            ui_sprite_barrage_module_name(module), UI_SPRITE_BARRAGE_OBJ_NAME);
        return -1;
    }

    ui_sprite_component_meta_set_init_fun(meta, ui_sprite_barrage_obj_init, module);
    ui_sprite_component_meta_set_copy_fun(meta, ui_sprite_barrage_obj_copy, module);
    ui_sprite_component_meta_set_enter_fun(meta, ui_sprite_barrage_obj_enter, module);
    ui_sprite_component_meta_set_exit_fun(meta, ui_sprite_barrage_obj_exit, module);
    ui_sprite_component_meta_set_free_fun(meta, ui_sprite_barrage_obj_fini, module);

    if (ui_sprite_cfg_loader_add_comp_loader(
            module->m_loader, UI_SPRITE_BARRAGE_OBJ_NAME, ui_sprite_barrage_obj_load, module)
        != 0)
    {
        CPE_ERROR(
            module->m_em, "%s: %s register: register loader fail",
            ui_sprite_barrage_module_name(module), UI_SPRITE_BARRAGE_OBJ_NAME);
        ui_sprite_component_meta_free(meta);
        return -1;
    }

    return 0;
}

void ui_sprite_barrage_obj_unregist(ui_sprite_barrage_module_t module) {
    ui_sprite_component_meta_t meta;

    if ((meta = ui_sprite_component_meta_find(module->m_repo, UI_SPRITE_BARRAGE_OBJ_NAME))) {
        ui_sprite_component_meta_free(meta);
    }
    else {
        CPE_ERROR(
            module->m_em, "%s: %s unregister: meta not exist",
            ui_sprite_barrage_module_name(module), UI_SPRITE_BARRAGE_OBJ_NAME);
    }

    if (ui_sprite_cfg_loader_remove_comp_loader(module->m_loader, UI_SPRITE_BARRAGE_OBJ_NAME) != 0) {
        CPE_ERROR(
            module->m_em, "%s: %s unregister: loader remove fail",
            ui_sprite_barrage_module_name(module), UI_SPRITE_BARRAGE_OBJ_NAME);
    }
}

const char * UI_SPRITE_BARRAGE_OBJ_NAME = "BarrageObj";

#ifdef __cplusplus
}
#endif
