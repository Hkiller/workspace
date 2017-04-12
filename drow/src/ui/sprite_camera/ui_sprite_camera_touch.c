#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/math_ex.h"
#include "cpe/cfg/cfg_read.h"
#include "render/utils/ui_transform.h"
#include "ui/sprite/ui_sprite_world.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_meta.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui/sprite_cfg/ui_sprite_cfg_loader.h"
#include "ui_sprite_camera_touch_i.h"
#include "ui_sprite_camera_utils.h"
#include "protocol/ui/sprite_camera/ui_sprite_camera_evt.h"

ui_sprite_camera_touch_t ui_sprite_camera_touch_create(ui_sprite_fsm_state_t fsm_state, const char * name) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_create(fsm_state, name, UI_SPRITE_CAMERA_TOUCH_NAME);
    return fsm_action ? ui_sprite_fsm_action_data(fsm_action) : NULL;
}

void ui_sprite_camera_touch_free(ui_sprite_camera_touch_t touch) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_from_data(touch);
    ui_sprite_fsm_action_free(fsm_action);
}

int ui_sprite_camera_touch_set_decorator(ui_sprite_camera_touch_t touch, const char * decorator) {
    return ui_percent_decorator_setup(&touch->m_updator.m_decorator, decorator, touch->m_module->m_em);
}

static void ui_sprite_camera_touch_on_begin(void * ctx, ui_sprite_event_t evt) {
    ui_sprite_camera_touch_t touch = ctx;
    ui_sprite_camera_module_t module = touch->m_module;
    ui_sprite_fsm_action_t action = ui_sprite_fsm_action_from_data(ctx);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(action);
    ui_sprite_world_t world = ui_sprite_entity_world(entity);
    ui_sprite_camera_env_t camera = ui_sprite_camera_env_find(world);
    ui_transform_t camera_transform;
    ui_vector_2 camera_pos;
    ui_vector_2 camera_scale;
    
    if (camera == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): camera-touch: on-begin: world no camera!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return;
    }

    camera_transform = ui_sprite_render_env_transform(camera->m_render);
    ui_transform_get_pos_2(camera_transform, &camera_pos);
    camera_scale.x = camera_transform->m_s.x;
    camera_scale.y = camera_transform->m_s.y;
    
    ui_sprite_camera_updator_stop(&touch->m_updator, camera);

    touch->m_state = ui_sprite_camera_touch_state_idle;

    ui_sprite_camera_screen_world_rect(
        &touch->m_init_camera_rect, &g_full_screen_percent, camera, &camera_pos, &camera_scale);

    ui_sprite_fsm_action_sync_update(action, 1);
}

extern void ui_sprite_camera_touch_on_scale(void * ctx, ui_sprite_event_t evt);

static void ui_sprite_camera_touch_on_move(void * ctx, ui_sprite_event_t evt) {
    ui_sprite_camera_touch_t touch = ctx;
    UI_SPRITE_EVT_CAMERA_TOUCH_MOVE const * evt_data = evt->data;
    ui_sprite_fsm_action_t action = ui_sprite_fsm_action_from_data(ctx);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(action);
    ui_sprite_world_t world = ui_sprite_entity_world(entity);
    ui_sprite_camera_env_t camera = ui_sprite_camera_env_find(world);
    ui_transform_t camera_transform = ui_sprite_render_env_transform(camera->m_render);
    ui_vector_2 camera_pos;
    ui_vector_2 delta = UI_VECTOR_2_INITLIZER(
        evt_data->new_screen_pos.x - evt_data->old_screen_pos.x,
        evt_data->new_screen_pos.y - evt_data->old_screen_pos.y);
    ui_vector_2 target_pos;
    ui_vector_2 target_scale;

    ui_transform_get_pos_2(camera_transform, &camera_pos);
    
    delta = ui_sprite_render_env_screen_to_world(camera->m_render, &delta);
    
    if (!ui_sprite_camera_updator_is_runing(&touch->m_updator)) {
        /*启动一次新的操作 */
        target_pos.x = camera_pos.x - delta.x;
        target_pos.y = camera_pos.y - delta.y;
    }
    else {
        /*在上一次操作基础上叠加 */
        target_pos.x = touch->m_updator.m_target_rect.lt.x - delta.x;
        target_pos.y = touch->m_updator.m_target_rect.lt.y - delta.y;
    }

    target_scale.x = camera_transform->m_s.x;
    target_scale.y = camera_transform->m_s.y;

    ui_sprite_camera_env_adj_camera_in_limit(camera, &target_pos, &target_scale);

    touch->m_state = ui_sprite_camera_touch_state_move;

    ui_sprite_camera_updator_set_max_speed(&touch->m_updator, evt_data->max_speed);

    ui_sprite_camera_updator_set_camera(&touch->m_updator, camera, target_pos, &target_scale);
}

static void ui_sprite_camera_touch_on_end(void * ctx, ui_sprite_event_t evt) {
    ui_sprite_camera_touch_t touch = ctx;
    ui_sprite_camera_module_t module = touch->m_module;
    UI_SPRITE_EVT_CAMERA_TOUCH_END const * evt_data = evt->data;
    ui_sprite_fsm_action_t action = ui_sprite_fsm_action_from_data(ctx);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(action);
    ui_sprite_world_t world = ui_sprite_entity_world(entity);
    ui_sprite_camera_env_t camera = ui_sprite_camera_env_find(world);
    ui_transform_t camera_transform = ui_sprite_render_env_transform(camera->m_render);
    ui_vector_2 camera_pos;
    ui_vector_2 camera_scale;
    
    ui_transform_get_pos_2(camera_transform, &camera_pos);
    camera_scale.x = camera_transform->m_s.x;
    camera_scale.y = camera_transform->m_s.y;
    
    ui_sprite_camera_updator_stop(&touch->m_updator, camera);

    if (camera == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): camera-touch: on-begin: world no camera!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        ui_sprite_fsm_action_sync_update(action, 0);
        return;
    }

    if (evt_data->speed_reduce == 0) {
        ui_sprite_fsm_action_sync_update(action, 0);

        if (ui_sprite_entity_debug(entity)) {
            CPE_INFO(
                module->m_em, "entity %d(%s): camera-touch: on-end: stop for no speed reduce!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        }
    }
    else {
        float speed = cpe_math_distance(0.0f, 0.0f, evt_data->speed.x, evt_data->speed.y);
        if (speed <= 0.001f) {
            ui_sprite_fsm_action_sync_update(action, 0);

            if (ui_sprite_entity_debug(entity)) {
                CPE_INFO(
                    module->m_em, "entity %d(%s): camera-touch: on-end: stop for no speed!",
                    ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
            }
        }
        else {
            ui_vector_2 target_pos;
            float radians;
            float duration;

            radians = cpe_math_radians(0.0f, 0.0f, evt_data->speed.x, evt_data->speed.y);

            if (evt_data->speed_adj_min < evt_data->speed_adj_max) {
                if (speed < evt_data->speed_adj_min) speed = evt_data->speed_adj_min;
                if (speed > evt_data->speed_adj_max) speed = evt_data->speed_adj_max;
            }

            duration =  speed / evt_data->speed_reduce;

            target_pos.x = camera_pos.x - speed * duration * cos(radians);
            target_pos.y = camera_pos.y - speed * duration * sin(radians);

            ui_sprite_camera_updator_set_max_speed(&touch->m_updator, speed);
            ui_sprite_camera_updator_set_camera(&touch->m_updator, camera, target_pos, &camera_scale);

            touch->m_state = ui_sprite_camera_touch_state_move_by_speed;

            if (ui_sprite_entity_debug(entity)) {
                CPE_INFO(
                    module->m_em, "entity %d(%s): camera-touch: on-end: move by speed, speed=%f, duration=%f!",
                    ui_sprite_entity_id(entity), ui_sprite_entity_name(entity),
                    speed, duration);
            }
        }
    }
}

static int ui_sprite_camera_touch_enter(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_camera_touch_t touch = ui_sprite_fsm_action_data(fsm_action); 
    ui_sprite_camera_module_t module = ctx;
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);

    if ((ui_sprite_fsm_action_add_event_handler(
             fsm_action, ui_sprite_event_scope_self, 
             "ui_sprite_evt_camera_touch_begin",
             ui_sprite_camera_touch_on_begin, touch) != 0)
        ||
        (ui_sprite_fsm_action_add_event_handler(
             fsm_action, ui_sprite_event_scope_self, 
             "ui_sprite_evt_camera_touch_move",
             ui_sprite_camera_touch_on_move, touch) != 0)
        ||
        (ui_sprite_fsm_action_add_event_handler(
             fsm_action, ui_sprite_event_scope_self, 
             "ui_sprite_evt_camera_touch_scale",
             ui_sprite_camera_touch_on_scale, touch) != 0)
        ||
        (ui_sprite_fsm_action_add_event_handler(
            fsm_action, ui_sprite_event_scope_self, 
            "ui_sprite_evt_camera_touch_end",
            ui_sprite_camera_touch_on_end, touch) != 0)
        )
        
    {
        CPE_ERROR(
            module->m_em, "entity %d(%s): camera-run-op: enter: add eventer handler fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }

    return 0;
}

static void ui_sprite_camera_touch_exit(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_camera_touch_t touch = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_world_t world = ui_sprite_entity_world(entity);
    ui_sprite_camera_env_t camera = ui_sprite_camera_env_find(world);

    ui_sprite_camera_updator_stop(&touch->m_updator, camera);
}

static int ui_sprite_camera_touch_init(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_camera_touch_t touch = ui_sprite_fsm_action_data(fsm_action);

    bzero(touch, sizeof(*touch));

    touch->m_module = ctx;

    return 0;
}

static void ui_sprite_camera_touch_clear(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_camera_touch_t touch = ui_sprite_fsm_action_data(fsm_action);

    assert(touch->m_updator.m_curent_op_id == 0);
}

static int ui_sprite_camera_touch_copy(ui_sprite_fsm_action_t to, ui_sprite_fsm_action_t from, void * ctx) {
    if (ui_sprite_camera_touch_init(to, ctx)) return -1;

    return 0;
}

static void ui_sprite_camera_touch_update(ui_sprite_fsm_action_t fsm_action, void * ctx, float delta) {
    ui_sprite_camera_touch_t touch = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_camera_module_t module = ctx;
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_world_t world = ui_sprite_entity_world(entity);
    ui_sprite_camera_env_t camera = ui_sprite_camera_env_find(world);

    if (touch->m_state == ui_sprite_camera_touch_state_idle) return;

    if (camera == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): camera touch: no camera!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        ui_sprite_fsm_action_stop_update(fsm_action);
        return;
    }

    ui_sprite_camera_updator_update(&touch->m_updator, camera, delta);

    if (!ui_sprite_camera_updator_is_runing(&touch->m_updator)) {
        if (touch->m_state == ui_sprite_camera_touch_state_move_by_speed) {
            if (ui_sprite_entity_debug(entity)) {
                CPE_INFO(
                    module->m_em, "entity %d(%s): camera-touch: update: move by speed stop for done!",
                    ui_sprite_entity_id(entity), ui_sprite_entity_name(entity))
                    }

            ui_sprite_fsm_action_stop_update(fsm_action);
        }
        else {
            touch->m_state = ui_sprite_camera_touch_state_idle;
        }
    }
}

static ui_sprite_fsm_action_t ui_sprite_camera_touch_load(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg) {
    ui_sprite_camera_module_t module = ctx;
    ui_sprite_camera_touch_t camera_camera_touch = ui_sprite_camera_touch_create(fsm_state, name);
    const char * decorator;

    if (camera_camera_touch == NULL) {
        CPE_ERROR(module->m_em, "%s: create camera_camera_touch action: create fail!", ui_sprite_camera_module_name(module));
        return NULL;
    }

    if ((decorator = cfg_get_string(cfg, "decorator", NULL))) {
        if (ui_sprite_camera_touch_set_decorator(camera_camera_touch, decorator) != 0) {
            CPE_ERROR(module->m_em, "%s: create camera_camera_touch action: create fail!", ui_sprite_camera_module_name(module));
            ui_sprite_camera_touch_free(camera_camera_touch);
            return NULL;
        }
    }

    return ui_sprite_fsm_action_from_data(camera_camera_touch);
}

int ui_sprite_camera_touch_regist(ui_sprite_camera_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_create(module->m_fsm_module, UI_SPRITE_CAMERA_TOUCH_NAME, sizeof(struct ui_sprite_camera_touch));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: camera camera touch register: meta create fail",
            ui_sprite_camera_module_name(module));
        return -1;
    }

    ui_sprite_fsm_action_meta_set_enter_fun(meta, ui_sprite_camera_touch_enter, module);
    ui_sprite_fsm_action_meta_set_exit_fun(meta, ui_sprite_camera_touch_exit, module);
    ui_sprite_fsm_action_meta_set_init_fun(meta, ui_sprite_camera_touch_init, module);
    ui_sprite_fsm_action_meta_set_copy_fun(meta, ui_sprite_camera_touch_copy, module);
    ui_sprite_fsm_action_meta_set_free_fun(meta, ui_sprite_camera_touch_clear, module);
	ui_sprite_fsm_action_meta_set_update_fun(meta, ui_sprite_camera_touch_update, module);

    if (module->m_loader) {
        if (ui_sprite_cfg_loader_add_action_loader(module->m_loader, UI_SPRITE_CAMERA_TOUCH_NAME, ui_sprite_camera_touch_load, module) != 0) {
            ui_sprite_fsm_action_meta_free(meta);
            return -1;
        }
    }
    
    return 0;
}

void ui_sprite_camera_touch_unregist(ui_sprite_camera_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_find(module->m_fsm_module, UI_SPRITE_CAMERA_TOUCH_NAME);
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: camera camera touch unregister: meta not exist",
            ui_sprite_camera_module_name(module));
        return;
    }

    ui_sprite_fsm_action_meta_free(meta);

    if (module->m_loader) {
        ui_sprite_cfg_loader_remove_action_loader(module->m_loader, UI_SPRITE_CAMERA_TOUCH_NAME);
    }
}

const char * UI_SPRITE_CAMERA_TOUCH_NAME = "camera-touch";

