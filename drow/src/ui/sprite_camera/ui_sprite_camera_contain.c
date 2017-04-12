#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/math_ex.h"
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "render/utils/ui_transform.h"
#include "ui/sprite/ui_sprite_world.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite/ui_sprite_group.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_meta.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui/sprite_cfg/ui_sprite_cfg_loader.h"
#include "ui/sprite_2d/ui_sprite_2d_utils.h"
#include "ui/sprite_2d/ui_sprite_2d_transform.h"
#include "ui_sprite_camera_contain_i.h"
#include "ui_sprite_camera_utils.h"
#include "protocol/ui/sprite_camera/ui_sprite_camera_evt.h"

ui_sprite_camera_contain_t ui_sprite_camera_contain_create(ui_sprite_fsm_state_t fsm_state, const char * name) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_create(fsm_state, name, UI_SPRITE_CAMERA_CONTAIN_NAME);
    return fsm_action ? ui_sprite_fsm_action_data(fsm_action) : NULL;
}

void ui_sprite_camera_contain_free(ui_sprite_camera_contain_t contain) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_from_data(contain);
    ui_sprite_fsm_action_free(fsm_action);
}

int ui_sprite_camera_contain_set_decorator(ui_sprite_camera_contain_t contain, const char * decorator) {
    return ui_percent_decorator_setup(&contain->m_updator.m_decorator, decorator, contain->m_module->m_em);
}

static int ui_sprite_camera_contain_put_rect_by_x(
    ui_sprite_camera_contain_t contain, ui_sprite_camera_env_t camera, 
    ui_vector_2_t screen_size, ui_vector_2 * pos, ui_vector_2_t scale,
    ui_rect const * check, ui_rect * screen_rect)
{
    /* uint8_t lock_lt = 0; */
    /* uint8_t lock_rb = 0; */

    if (check->lt.x < screen_rect->lt.x) {
        float diff_scale = (screen_rect->lt.x - check->lt.x) / screen_size->x;
        if ((scale->x + diff_scale) < contain->m_max_scale) {
            scale->x += diff_scale;
            pos->x = check->lt.x;
            pos->y = ui_sprite_camera_env_trace_x2y(camera, pos->x, scale);
            ui_sprite_camera_screen_world_rect(screen_rect, &contain->m_screen_rect, camera, pos, scale);
        }
        else {
            float move_x;

            if (scale->x < contain->m_max_scale) {
                pos->x -= screen_size->x * (contain->m_max_scale - scale->x);
                scale->x = contain->m_max_scale;
                pos->y = ui_sprite_camera_env_trace_x2y(camera, pos->x, scale);
                ui_sprite_camera_screen_world_rect(screen_rect, &contain->m_screen_rect, camera, pos, scale);
            }

            assert(check->lt.x < screen_rect->lt.x);
            move_x = screen_rect->lt.x - check->lt.x;

            if (screen_rect->rb.x - move_x > check->rb.x) {
                pos->x -= move_x;
                pos->y = ui_sprite_camera_env_trace_x2y(camera, pos->x, scale);
                ui_sprite_camera_screen_world_rect(screen_rect, &contain->m_screen_rect, camera, pos, scale);
            }
            else {
                return -1;
            }
        }
    }

    if (check->rb.x > screen_rect->rb.x) {
        float diff_scale = (check->rb.x - screen_rect->rb.x) / screen_size->x;
        if ((scale->x + diff_scale) < contain->m_max_scale) {
            scale->x += diff_scale;
            pos->y = ui_sprite_camera_env_trace_x2y(camera, pos->x, scale);
            ui_sprite_camera_screen_world_rect(screen_rect, &contain->m_screen_rect, camera, pos, scale);
        }
        else {
            float move_x;

            if (scale->x < contain->m_max_scale) {
                scale->x = contain->m_max_scale;
                pos->y = ui_sprite_camera_env_trace_x2y(camera, pos->x, scale);
                ui_sprite_camera_screen_world_rect(screen_rect, &contain->m_screen_rect, camera, pos, scale);
            }

            move_x = check->rb.x - screen_rect->rb.x;

            if (pos->x + move_x < check->lt.x) {
                pos->x += move_x;
                pos->y = ui_sprite_camera_env_trace_x2y(camera, pos->x, scale);
                ui_sprite_camera_screen_world_rect(screen_rect, &contain->m_screen_rect, camera, pos, scale);
            }
            else {
                return -1;
            }
        }
    }

    return 0;
}

static int ui_sprite_camera_contain_put_rect_no_track(
    ui_sprite_camera_contain_t contain, ui_sprite_camera_env_t camera, ui_vector_2_t screen_size, 
    ui_vector_2 * pos, ui_vector_2_t scale,
    ui_rect const * check, ui_rect * screen_rect)
{
    if (check->lt.x < screen_rect->lt.x) {
        float diff_scale = (screen_rect->lt.x - check->lt.x) / screen_size->x;
        if ((scale->x + diff_scale) < contain->m_max_scale) {
            scale->x += diff_scale;
            pos->x = check->lt.x;
            ui_sprite_camera_screen_world_rect(screen_rect, &contain->m_screen_rect, camera, pos, scale);
        }
        else {
            float move_x;

            if (scale->x < contain->m_max_scale) {
                pos->x -= screen_size->x * (contain->m_max_scale - scale->x);
                scale->x = contain->m_max_scale;
                ui_sprite_camera_screen_world_rect(screen_rect, &contain->m_screen_rect, camera, pos, scale);
            }

            assert(check->lt.x < screen_rect->lt.x);
            move_x = screen_rect->lt.x - check->lt.x;

            if (screen_rect->rb.x - move_x > check->rb.x) {
                pos->x -= move_x;
                ui_sprite_camera_screen_world_rect(screen_rect, &contain->m_screen_rect, camera, pos, scale);
            }
            else {
                return -1;
            }
        }
    }

    if (check->lt.y < screen_rect->lt.y) {
        float diff_scale = (screen_rect->lt.y - check->lt.y) / screen_size->y;
        if ((scale->y + diff_scale) < contain->m_max_scale) {
            scale->y += diff_scale;
            pos->y = check->lt.y;
            ui_sprite_camera_screen_world_rect(screen_rect, &contain->m_screen_rect, camera, pos, scale);
        }
        else {
            float move_y;

            if (scale->y < contain->m_max_scale) {
                pos->y -= screen_size->y * (contain->m_max_scale - scale->y);
                scale->y = contain->m_max_scale;
                ui_sprite_camera_screen_world_rect(
                    screen_rect, &contain->m_screen_rect, camera, pos, scale);
            }

            assert(check->lt.y < screen_rect->lt.y);
            move_y = screen_rect->lt.y - check->lt.y;

            if (screen_rect->rb.y - move_y > check->rb.y) {
                pos->y -= move_y;
                ui_sprite_camera_screen_world_rect(
                    screen_rect, &contain->m_screen_rect, camera, pos, scale);
            }
            else {
                return -1;
            }
        }
    }

    if (check->rb.x > screen_rect->rb.x) {
        float diff_scale = (check->rb.x - screen_rect->rb.x) / screen_size->x;
        if ((scale->x + diff_scale) < contain->m_max_scale) {
            scale->x += diff_scale;
            ui_sprite_camera_screen_world_rect(screen_rect, &contain->m_screen_rect, camera, pos, scale);
        }
        else {
            float move_x;

            if (scale->x < contain->m_max_scale) {
                scale->x = contain->m_max_scale;
                ui_sprite_camera_screen_world_rect(screen_rect, &contain->m_screen_rect, camera, pos, scale);
            }

            move_x = check->rb.x - screen_rect->rb.x;

            if (pos->x + move_x < check->lt.x) {
                pos->x += move_x;
                ui_sprite_camera_screen_world_rect(screen_rect, &contain->m_screen_rect, camera, pos, scale);
            }
            else {
                return -1;
            }
        }
    }

    if (check->rb.y > screen_rect->rb.y) {
        float diff_scale = (check->rb.y - screen_rect->rb.y) / screen_size->y;
        if ((scale->y + diff_scale) < contain->m_max_scale) {
            scale->y += diff_scale;
            ui_sprite_camera_screen_world_rect(screen_rect, &contain->m_screen_rect, camera, pos, scale);
        }
        else {
            float move_y;

            if (scale->y < contain->m_max_scale) {
                scale->y = contain->m_max_scale;
                ui_sprite_camera_screen_world_rect(screen_rect, &contain->m_screen_rect, camera, pos, scale);
            }

            move_y = check->rb.y - screen_rect->rb.y;

            if (pos->y + move_y < check->lt.y) {
                pos->y += move_y;
                ui_sprite_camera_screen_world_rect(screen_rect, &contain->m_screen_rect, camera, pos, scale);
            }
            else {
                return -1;
            }
        }
    }

    return 0;
}

static int ui_sprite_camera_contain_calc_target_camera(
    ui_sprite_camera_contain_t contain, ui_sprite_camera_env_t camera, ui_vector_2_t screen_size, ui_sprite_entity_t entity, 
    ui_vector_2_t target_camera_pos, ui_vector_2_t target_scale, ui_vector_2 contain_world_lt, ui_vector_2 contain_world_rb)
{
    ui_sprite_camera_module_t module = contain->m_module;
	ui_sprite_world_t world = ui_sprite_entity_world(entity);
    ui_sprite_group_t group;
    ui_sprite_entity_it_t target_entity_it;
    ui_sprite_entity_t target_entity;
    ui_rect screen_world_rect;
    ui_rect check_rect;
    ui_transform_t camera_trans = ui_sprite_render_env_transform(camera->m_render);

    if (contain->m_group_name[0]) {
        group = ui_sprite_group_find_by_name(world,  contain->m_group_name);
        if (group == NULL) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): camera contain: group %s not exist!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), contain->m_group_name);
            return -1;
        }
    }
    else {
        group = ui_sprite_group_find_by_id(world,  contain->m_group_id);
        if (group == NULL) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): camera contain: group %d not exist!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), contain->m_group_id);
            return -1;
        }
    }

    target_entity_it = ui_sprite_group_entities(module->m_alloc, group);
    if (target_entity_it == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): camera contain: group %s get entities fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), contain->m_group_name);
        return -1;
    }

    if (contain->m_best_scale) {
        target_scale->x = contain->m_best_scale;
        target_scale->y = contain->m_best_scale;
    }
    else {
        target_scale->x = camera_trans->m_s.x;
        target_scale->y = camera_trans->m_s.y;
    }
    
    ui_transform_get_pos_2(camera_trans, target_camera_pos);

    ui_sprite_camera_screen_world_rect(
        &screen_world_rect, &contain->m_screen_rect, camera, target_camera_pos, target_scale);
    bzero(&check_rect, sizeof(check_rect));

    while((target_entity = ui_sprite_entity_it_next(target_entity_it))) {
        ui_sprite_2d_transform_t transform = ui_sprite_2d_transform_find(target_entity);
        ui_rect entity_rect;
        int put_rect_r;
        ui_vector_2 new_pos;
        ui_vector_2 new_scale;
        ui_rect new_check_rect;

        if (transform == NULL) continue;

        entity_rect.lt = ui_sprite_2d_transform_world_pos(
            transform,
            UI_SPRITE_2D_TRANSFORM_POS_F_TOP_LEFT, 
            UI_SPRITE_2D_TRANSFORM_POS_ADJ_BY_FLIP | UI_SPRITE_2D_TRANSFORM_POS_ADJ_BY_SCALE);

        entity_rect.rb = ui_sprite_2d_transform_world_pos(
            transform,
            UI_SPRITE_2D_TRANSFORM_POS_F_BOTTOM_RIGHT, 
            UI_SPRITE_2D_TRANSFORM_POS_ADJ_BY_FLIP | UI_SPRITE_2D_TRANSFORM_POS_ADJ_BY_SCALE);

        if (entity_rect.lt.x > entity_rect.rb.x || entity_rect.lt.y > entity_rect.rb.y) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): camera contain: entitie %d(%s) rect (%f,%f)-(%f,%f) error!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity),
                ui_sprite_entity_id(target_entity), ui_sprite_entity_name(target_entity),
                entity_rect.lt.x, entity_rect.lt.y, entity_rect.rb.x, entity_rect.rb.y);
            continue;
        }
        
        /* printf("camera=(%f,%f)-%f\n", target_camera_pos->x, target_camera_pos->y, *target_scale); */
        /* printf("entity_rect=(%f,%f)-(%f,%f)\n", entity_rect.lt.x, entity_rect.lt.y, entity_rect.rb.x, entity_rect.rb.y); */
        /* printf("world_rect=(%f,%f)-(%f,%f)\n", screen_world_rect.lt.x, screen_world_rect.lt.y, screen_world_rect.rb.x, screen_world_rect.rb.y); */

        if (ui_sprite_2d_rect_in_rect(&entity_rect, &screen_world_rect)) continue;

        new_pos = *target_camera_pos;
        new_scale = *target_scale;
        new_check_rect = check_rect;
        ui_sprite_2d_rect_merge(&new_check_rect, &entity_rect);

        switch(camera->m_trace_type) {
        case ui_sprite_camera_trace_none:
            put_rect_r = ui_sprite_camera_contain_put_rect_no_track(
                contain, camera, screen_size, &new_pos, &new_scale, &new_check_rect, &screen_world_rect);
            break;
        case ui_sprite_camera_trace_by_x:
            put_rect_r = ui_sprite_camera_contain_put_rect_by_x(
                contain, camera, screen_size, &new_pos, &new_scale, &new_check_rect, &screen_world_rect);
            break;
        case ui_sprite_camera_trace_by_y:
            assert(0);
            put_rect_r = -1;
            break;
        default:
            assert(0);
            put_rect_r = -1;
            break;
        }

        if (put_rect_r == 0) {
            *target_camera_pos = new_pos;
            *target_scale = new_scale;
            check_rect = new_check_rect;
        }
    }

    ui_sprite_entity_it_free(target_entity_it);

    return 0;
}

static void ui_sprite_camera_contain_on_contain_group(void * ctx, ui_sprite_event_t evt) {
    ui_sprite_camera_contain_t contain = ctx;
    ui_sprite_camera_module_t module = contain->m_module;
	ui_sprite_fsm_action_t action = ui_sprite_fsm_action_from_data(ctx);
	ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(action);
	ui_sprite_world_t world = ui_sprite_entity_world(entity);
	ui_sprite_camera_env_t camera = ui_sprite_camera_env_find(world);
	UI_SPRITE_EVT_CAMERA_CONTAIN_GROUP const * evt_data = evt->data;

    ui_sprite_camera_updator_stop(&contain->m_updator, camera);

    if (evt_data->screen_lt.x > evt_data->screen_rb.x || evt_data->screen_lt.y > evt_data->screen_rb.y) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): camera on contain group: screen rect (%f,%f) - (%f,%f) error!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), 
            evt_data->screen_lt.x, evt_data->screen_lt.y, evt_data->screen_rb.x, evt_data->screen_rb.y);
        ui_sprite_fsm_action_sync_update(action, 0);
        return;
    }

    if (evt_data->screen_lt.x == evt_data->screen_rb.x || evt_data->screen_lt.y == evt_data->screen_rb.y) {
        contain->m_screen_rect.lt.x = 0.0f;
        contain->m_screen_rect.lt.y = 0.0f;
        contain->m_screen_rect.rb.x = 1.0f;
        contain->m_screen_rect.rb.y = 1.0f;
    }
    else {
        contain->m_screen_rect.lt.x = evt_data->screen_lt.x;
        contain->m_screen_rect.lt.y = evt_data->screen_lt.y;
        contain->m_screen_rect.rb.x = evt_data->screen_rb.x;
        contain->m_screen_rect.rb.y = evt_data->screen_rb.y;
    }

    contain->m_group_id = evt_data->group_id;
    cpe_str_dup(contain->m_group_name, sizeof(contain->m_group_name), evt_data->group_name);

    contain->m_max_scale = evt_data->max_scale;
    contain->m_best_scale = evt_data->best_scale;

    ui_sprite_camera_updator_stop(&contain->m_updator, camera);
    ui_sprite_camera_updator_set_max_speed(&contain->m_updator, evt_data->max_speed);
    
    ui_sprite_fsm_action_sync_update(action, 1);
}

static int ui_sprite_camera_contain_enter(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_camera_contain_t contain = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_camera_module_t module = ctx;

	if (ui_sprite_fsm_action_add_event_handler(
            fsm_action, ui_sprite_event_scope_self, 
            "ui_sprite_evt_camera_contain_group", ui_sprite_camera_contain_on_contain_group, contain) != 0
        )
	{    
		CPE_ERROR(module->m_em, "camera contain enter: add eventer handler fail!");
		return -1;
	}

    ui_sprite_camera_updator_set_max_speed(&contain->m_updator, 0.0f);
	
    return 0;
}

static void ui_sprite_camera_contain_exit(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_camera_contain_t contain = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_camera_module_t module = ctx;
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_world_t world = ui_sprite_entity_world(entity);
    ui_sprite_camera_env_t camera = ui_sprite_camera_env_find(world);

    if (camera == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): camera contain exit: no camera!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return;
    }

    ui_sprite_camera_updator_stop(&contain->m_updator, camera);
}

static void ui_sprite_camera_contain_update(ui_sprite_fsm_action_t fsm_action, void * ctx, float delta) {
    ui_sprite_camera_contain_t contain = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_camera_module_t module = ctx;
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_world_t world = ui_sprite_entity_world(entity);
    ui_sprite_camera_env_t camera = ui_sprite_camera_env_find(world);
    ui_vector_2_t screen_size = ui_sprite_render_env_size(camera->m_render);
    ui_vector_2 target_camera_pos;
    ui_vector_2 target_scale;

    ui_vector_2 world_lt = UI_VECTOR_2_ZERO;
    ui_vector_2 world_rb = UI_VECTOR_2_ZERO;

    if (camera == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): camera contain: update: no camera!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        goto CONTAIN_STOP_UPDATE;
    }

    if (ui_sprite_camera_contain_calc_target_camera(
            contain, camera, screen_size, entity, &target_camera_pos, &target_scale, world_lt, world_rb)
        != 0)
    {
        return;
    }

    ui_sprite_camera_updator_set_camera(&contain->m_updator, camera, target_camera_pos, &target_scale);

    ui_sprite_camera_updator_update(&contain->m_updator, camera, delta);

    return;

CONTAIN_STOP_UPDATE:
    ui_sprite_camera_updator_stop(&contain->m_updator, camera);
    ui_sprite_fsm_action_stop_update(fsm_action);
}

static int ui_sprite_camera_contain_init(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_camera_contain_t contain = ui_sprite_fsm_action_data(fsm_action);

    bzero(contain, sizeof(*contain));

    contain->m_module = ctx;

    return 0;
}

static void ui_sprite_camera_contain_clear(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_camera_contain_t contain = ui_sprite_fsm_action_data(fsm_action);

    assert(contain->m_updator.m_curent_op_id == 0);
}

static int ui_sprite_camera_contain_copy(ui_sprite_fsm_action_t to, ui_sprite_fsm_action_t from, void * ctx) {
    if (ui_sprite_camera_contain_init(to, ctx)) return -1;
    return 0;
}

static ui_sprite_fsm_action_t
ui_sprite_camera_contain_load(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg)
{
    ui_sprite_camera_module_t module = ctx;
    ui_sprite_camera_contain_t camera_camera_contain = ui_sprite_camera_contain_create(fsm_state, name);
    const char * decorator;

    if (camera_camera_contain == NULL) {
        CPE_ERROR(module->m_em, "%s: create camera_camera_contain action: create fail!", ui_sprite_camera_module_name(module));
        return NULL;
    }

    if ((decorator = cfg_get_string(cfg, "decorator", NULL))) {
        if (ui_sprite_camera_contain_set_decorator(camera_camera_contain, decorator) != 0) {
            CPE_ERROR(module->m_em, "%s: create camera_camera_contain action: create fail!", ui_sprite_camera_module_name(module));
            ui_sprite_camera_contain_free(camera_camera_contain);
            return NULL;
        }
    }

    return ui_sprite_fsm_action_from_data(camera_camera_contain);
}


int ui_sprite_camera_contain_regist(ui_sprite_camera_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_create(module->m_fsm_module, UI_SPRITE_CAMERA_CONTAIN_NAME, sizeof(struct ui_sprite_camera_contain));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: camera camera contain register: meta create fail",
            ui_sprite_camera_module_name(module));
        return -1;
    }

    ui_sprite_fsm_action_meta_set_enter_fun(meta, ui_sprite_camera_contain_enter, module);
    ui_sprite_fsm_action_meta_set_exit_fun(meta, ui_sprite_camera_contain_exit, module);
    ui_sprite_fsm_action_meta_set_init_fun(meta, ui_sprite_camera_contain_init, module);
    ui_sprite_fsm_action_meta_set_copy_fun(meta, ui_sprite_camera_contain_copy, module);
    ui_sprite_fsm_action_meta_set_free_fun(meta, ui_sprite_camera_contain_clear, module);
    ui_sprite_fsm_action_meta_set_update_fun(meta, ui_sprite_camera_contain_update, module);

    if (module->m_loader) {
        if (ui_sprite_cfg_loader_add_action_loader(module->m_loader, UI_SPRITE_CAMERA_CONTAIN_NAME, ui_sprite_camera_contain_load, module) != 0) {
            ui_sprite_fsm_action_meta_free(meta);
            return -1;
        }
    }
    
    return 0;
}

void ui_sprite_camera_contain_unregist(ui_sprite_camera_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_find(module->m_fsm_module, UI_SPRITE_CAMERA_CONTAIN_NAME);
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: camera camera contain unregister: meta not exist",
            ui_sprite_camera_module_name(module));
        return;
    }

    ui_sprite_fsm_action_meta_free(meta);

    if (module->m_loader) {
        ui_sprite_cfg_loader_remove_action_loader(module->m_loader, UI_SPRITE_CAMERA_CONTAIN_NAME);
    }
}

const char * UI_SPRITE_CAMERA_CONTAIN_NAME = "camera-contain";

