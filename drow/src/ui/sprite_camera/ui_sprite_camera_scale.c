#include  <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/math_ex.h"
#include "cpe/cfg/cfg_read.h"
#include "render/utils/ui_transform.h"
#include "ui/sprite/ui_sprite_world.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite_2d/ui_sprite_2d_utils.h"
#include "ui/sprite_2d/ui_sprite_2d_transform.h"
#include "ui/sprite_cfg/ui_sprite_cfg_loader.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_meta.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui_sprite_camera_scale_i.h"
#include "ui_sprite_camera_utils.h"
#include "protocol/ui/sprite_camera/ui_sprite_camera_evt.h"

ui_sprite_camera_scale_t ui_sprite_camera_scale_create(ui_sprite_fsm_state_t fsm_state, const char * name) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_create(fsm_state, name, UI_SPRITE_CAMERA_SCALE_NAME);
    return fsm_action ? ui_sprite_fsm_action_data(fsm_action) : NULL;
}

void ui_sprite_camera_scale_free(ui_sprite_camera_scale_t scale) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_from_data(scale);
    ui_sprite_fsm_action_free(fsm_action);
}

int ui_sprite_camera_scale_set_decorator(ui_sprite_camera_scale_t scale, const char * decorator) {
    return ui_percent_decorator_setup(&scale->m_updator.m_decorator, decorator, scale->m_module->m_em);
}

static void ui_sprite_camera_scale_on_scale(void * ctx, ui_sprite_event_t evt) {
    ui_sprite_camera_scale_t scale = ctx;
    ui_sprite_camera_module_t module = scale->m_module;
	ui_sprite_fsm_action_t action = ui_sprite_fsm_action_from_data(ctx);
	ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(action);
	ui_sprite_world_t world = ui_sprite_entity_world(entity);
	ui_sprite_camera_env_t camera = ui_sprite_camera_env_find(world);
    ui_vector_2_t screen_size = ui_sprite_render_env_size(camera->m_render);
    ui_transform_t camera_transform = ui_sprite_render_env_transform(camera->m_render);
    ui_vector_2 camera_pos;
	UI_SPRITE_EVT_CAMERA_SCALE const * evt_data = evt->data;
    ui_vector_2 target_pos;
    ui_vector_2 target_scale;
    ui_sprite_entity_t lock_entity = NULL;

    ui_transform_get_pos_2(camera_transform, &camera_pos);
    
    ui_sprite_camera_updator_stop(&scale->m_updator, camera);

    if (cpe_float_cmp(camera_transform->m_s.x, evt_data->scale, 0.001) == 0
        && cpe_float_cmp(camera_transform->m_s.y, evt_data->scale, 0.001) == 0)
    {
        if (ui_sprite_entity_debug(entity)) {
            CPE_INFO(
                module->m_em, "entity %d(%s): camera on scale: no scale change, scale=%f!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), evt_data->scale);
        }
        ui_sprite_fsm_action_sync_update(action, 0);
        return;
    }

    target_scale.x = evt_data->scale;
    target_scale.y = evt_data->scale;    

    if (evt_data->lock_entity_id) {
        lock_entity = ui_sprite_entity_find_by_id(world, evt_data->lock_entity_id);
        if (lock_entity == NULL) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): camera on scale: lock entity %d not exist!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), evt_data->lock_entity_id);
            ui_sprite_fsm_action_sync_update(action, 0);
            return;
        }
    }
    else if (evt_data->lock_entity_name[0]) {
        lock_entity = ui_sprite_entity_find_by_name(world, evt_data->lock_entity_name);
        if (lock_entity == NULL) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): camera on scale: lock entity %s not exist!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), evt_data->lock_entity_name);
            ui_sprite_fsm_action_sync_update(action, 0);
            return;
        }
    }

    if (lock_entity) {
        ui_sprite_2d_transform_t transform;
        ui_vector_2 pos_in_screen;
        ui_vector_2 pos_in_world;

        transform = ui_sprite_2d_transform_find(lock_entity);
        if (transform == NULL) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): camera on scale: lock entity %d(%s) no transform!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity),
                ui_sprite_entity_id(lock_entity), ui_sprite_entity_name(lock_entity));
            ui_sprite_fsm_action_sync_update(action, 0);
            return;
        }

        pos_in_world = ui_sprite_2d_transform_world_pos(transform, UI_SPRITE_2D_TRANSFORM_POS_ORIGIN, 0);

        pos_in_screen.x = (pos_in_world.x - camera_pos.x) / (screen_size->x * camera_transform->m_s.x);
        pos_in_screen.y = (pos_in_world.y - camera_pos.y) / (screen_size->y * camera_transform->m_s.y);

        if (!ui_sprite_2d_pt_in_rect(pos_in_screen, &g_full_screen_percent)) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): camera on scale: lock pos (%f,%f) is not in screen!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity),
                pos_in_screen.x, pos_in_screen.y);
            ui_sprite_fsm_action_sync_update(action, 0);
            return;
        }

        target_pos.x = 
            camera_pos.x
            + (screen_size->x * (camera_transform->m_s.x - target_scale.x) * pos_in_screen.x);

        target_pos.y =
            camera_pos.y
            + (screen_size->y * (camera_transform->m_s.y - target_scale.y) * pos_in_screen.y);

        ui_sprite_camera_env_adj_camera_in_limit_with_lock_pos(
            camera, &target_pos, &target_scale, &pos_in_screen, &pos_in_world);
    }
    else {
        target_pos = camera_pos;
        ui_sprite_camera_env_adj_camera_in_limit(camera, &target_pos, &target_scale);
    }

    ui_sprite_camera_updator_set_max_speed(&scale->m_updator, evt_data->max_speed);

    ui_sprite_camera_updator_set_camera(&scale->m_updator, camera, target_pos, &target_scale);

    ui_sprite_fsm_action_sync_update(action, ui_sprite_camera_updator_is_runing(&scale->m_updator));
}

static int ui_sprite_camera_scale_enter(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_camera_scale_t scale = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_camera_module_t module = ctx;

	if (ui_sprite_fsm_action_add_event_handler(
		fsm_action, ui_sprite_event_scope_self, 
		"ui_sprite_evt_camera_scale", ui_sprite_camera_scale_on_scale, scale) != 0)
	{    
		CPE_ERROR(module->m_em, "camera scale enter: add eventer handler fail!");
		return -1;
	}
	
    return 0;
}

static void ui_sprite_camera_scale_exit(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_camera_scale_t scale = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_world_t world = ui_sprite_entity_world(entity);
    ui_sprite_camera_env_t camera = ui_sprite_camera_env_find(world);

    ui_sprite_camera_updator_stop(&scale->m_updator, camera);
}

static void ui_sprite_camera_scale_update(ui_sprite_fsm_action_t fsm_action, void * ctx, float delta) {
    ui_sprite_camera_scale_t scale = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_camera_module_t module = ctx;
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_world_t world = ui_sprite_entity_world(entity);
    ui_sprite_camera_env_t camera = ui_sprite_camera_env_find(world);

    if (camera == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): camera update: no camera!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        ui_sprite_camera_updator_stop(&scale->m_updator, camera);
        ui_sprite_fsm_action_stop_update(fsm_action);
        return;
    }

    ui_sprite_camera_updator_update(&scale->m_updator, camera, delta);

    if (!ui_sprite_camera_updator_is_runing(&scale->m_updator)) {
        ui_sprite_fsm_action_stop_update(fsm_action);
    }
}

static int ui_sprite_camera_scale_init(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_camera_scale_t scale = ui_sprite_fsm_action_data(fsm_action);

    bzero(scale, sizeof(*scale));

    scale->m_module = ctx;

    return 0;
}

static void ui_sprite_camera_scale_clear(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_camera_scale_t scale = ui_sprite_fsm_action_data(fsm_action);

    assert (scale->m_updator.m_curent_op_id == 0);
}

static int ui_sprite_camera_scale_copy(ui_sprite_fsm_action_t to, ui_sprite_fsm_action_t from, void * ctx) {
    if (ui_sprite_camera_scale_init(to, ctx)) return -1;
    return 0;
}

static ui_sprite_fsm_action_t ui_sprite_camera_scale_load(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg) {
    ui_sprite_camera_module_t module = ctx;
    ui_sprite_camera_scale_t camera_camera_scale = ui_sprite_camera_scale_create(fsm_state, name);
    const char * decorator;

    if (camera_camera_scale == NULL) {
        CPE_ERROR(module->m_em, "%s: create camera_camera_scale action: create fail!", ui_sprite_camera_module_name(module));
        return NULL;
    }

    if ((decorator = cfg_get_string(cfg, "decorator", NULL))) {
        if (ui_sprite_camera_scale_set_decorator(camera_camera_scale, decorator) != 0) {
            CPE_ERROR(module->m_em, "%s: create camera_camera_scale action: create fail!", ui_sprite_camera_module_name(module));
            ui_sprite_camera_scale_free(camera_camera_scale);
            return NULL;
        }
    }

    return ui_sprite_fsm_action_from_data(camera_camera_scale);
}

int ui_sprite_camera_scale_regist(ui_sprite_camera_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_create(module->m_fsm_module, UI_SPRITE_CAMERA_SCALE_NAME, sizeof(struct ui_sprite_camera_scale));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: camera camera scale register: meta create fail",
            ui_sprite_camera_module_name(module));
        return -1;
    }

    ui_sprite_fsm_action_meta_set_enter_fun(meta, ui_sprite_camera_scale_enter, module);
    ui_sprite_fsm_action_meta_set_exit_fun(meta, ui_sprite_camera_scale_exit, module);
    ui_sprite_fsm_action_meta_set_init_fun(meta, ui_sprite_camera_scale_init, module);
    ui_sprite_fsm_action_meta_set_copy_fun(meta, ui_sprite_camera_scale_copy, module);
    ui_sprite_fsm_action_meta_set_free_fun(meta, ui_sprite_camera_scale_clear, module);
    ui_sprite_fsm_action_meta_set_update_fun(meta, ui_sprite_camera_scale_update, module);

    if (module->m_loader) {
        if (ui_sprite_cfg_loader_add_action_loader(module->m_loader, UI_SPRITE_CAMERA_SCALE_NAME, ui_sprite_camera_scale_load, module) != 0) {
            ui_sprite_fsm_action_meta_free(meta);
            return -1;
        }
    }
    
    return 0;
}

void ui_sprite_camera_scale_unregist(ui_sprite_camera_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_find(module->m_fsm_module, UI_SPRITE_CAMERA_SCALE_NAME);
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: camera camera scale unregister: meta not exist",
            ui_sprite_camera_module_name(module));
        return;
    }

    ui_sprite_fsm_action_meta_free(meta);

    if (module->m_loader) {
        ui_sprite_cfg_loader_remove_action_loader(module->m_loader, UI_SPRITE_CAMERA_SCALE_NAME);
    }
}

const char * UI_SPRITE_CAMERA_SCALE_NAME = "camera-scale";

