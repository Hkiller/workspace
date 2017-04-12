#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/math_ex.h"
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
#include "ui_sprite_camera_move_i.h"
#include "ui_sprite_camera_env_i.h"
#include "protocol/ui/sprite_camera/ui_sprite_camera_evt.h"

ui_sprite_camera_move_t ui_sprite_camera_move_create(ui_sprite_fsm_state_t fsm_state, const char * name) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_create(fsm_state, name, UI_SPRITE_CAMERA_MOVE_NAME);
    return fsm_action ? ui_sprite_fsm_action_data(fsm_action) : NULL;
}

void ui_sprite_camera_move_free(ui_sprite_camera_move_t move) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_from_data(move);
    ui_sprite_fsm_action_free(fsm_action);
}

int ui_sprite_camera_move_set_decorator(ui_sprite_camera_move_t move, const char * decorator) {
    return ui_percent_decorator_setup(&move->m_updator.m_decorator, decorator, move->m_module->m_em);
}

static ui_vector_2
ui_sprite_camera_move_target_camera_pos(
ui_sprite_camera_move_t move, ui_sprite_camera_env_t camera,
    ui_vector_2 const * pos_in_screen, ui_vector_2 const * pos_in_world, ui_vector_2_t scale)
{
    ui_vector_2_t screen_size = ui_sprite_render_env_size(camera->m_render);
    ui_vector_2 pos;

    pos.x = pos_in_world->x - screen_size->x * scale->x * pos_in_screen->x;
    pos.y = pos_in_world->y - screen_size->y * scale->y * pos_in_screen->y;

    return pos;
}

static void ui_sprite_camera_move_on_move_to_entity(void * ctx, ui_sprite_event_t evt) {
    ui_sprite_camera_move_t move = ctx;
    ui_sprite_camera_module_t module = move->m_module;
	ui_sprite_fsm_action_t action = ui_sprite_fsm_action_from_data(ctx);
	ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(action);
	ui_sprite_world_t world = ui_sprite_entity_world(entity);
	ui_sprite_camera_env_t camera = ui_sprite_camera_env_find(world);
	UI_SPRITE_EVT_CAMERA_MOVE_TO_ENTITY const * evt_data = evt->data;
    ui_transform_t camera_transform = ui_sprite_render_env_transform(camera->m_render);
    uint8_t pos_of_entity;
    ui_vector_2 entity_pos_in_screen;
    ui_vector_2 entity_pos_in_world;
    ui_vector_2 target_pos;
    ui_vector_2 target_scale;

    ui_sprite_camera_updator_stop(&move->m_updator, camera);

    pos_of_entity = ui_sprite_2d_transform_pos_policy_from_str(evt_data->pos_of_entity);
    if (pos_of_entity == 0) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): camera on move to entity: pos of entity %s is unknown!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), evt_data->pos_of_entity);
        ui_sprite_fsm_action_sync_update(action, 0);
        return;
    }

    if (evt_data->scale == 0.0f) {
        target_scale.x = camera_transform->m_s.x;
        target_scale.y = camera_transform->m_s.y;
    }
    else {
        target_scale.x = evt_data->scale;
        target_scale.y = evt_data->scale;
    }
    
    if (ui_sprite_camera_env_pos_of_entity(&entity_pos_in_world, world, evt_data->entity_id, evt_data->entity_name, pos_of_entity) != 0) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): camera on set to entity: get pos of entity %d(%s) fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), evt_data->entity_id, evt_data->entity_name);
        ui_sprite_fsm_action_sync_update(action, 0);
        return;
    }

    entity_pos_in_screen.x = evt_data->pos_in_screen.x;
    entity_pos_in_screen.y = evt_data->pos_in_screen.y;

    target_pos =
        ui_sprite_camera_move_target_camera_pos(
            move, camera, &entity_pos_in_screen, &entity_pos_in_world, &target_scale);

    ui_sprite_camera_env_adj_camera_in_limit(camera, &target_pos, &target_scale);

    ui_sprite_camera_updator_set_max_speed(&move->m_updator, evt_data->max_speed);

    ui_sprite_camera_updator_set_camera(&move->m_updator, camera, target_pos, &target_scale);

    ui_sprite_fsm_action_sync_update(action, ui_sprite_camera_updator_is_runing(&move->m_updator));
}

static void ui_sprite_camera_move_on_move_to_group(void * ctx, ui_sprite_event_t evt) {
    ui_sprite_camera_move_t move = ctx;
    ui_sprite_camera_module_t module = move->m_module;
	ui_sprite_fsm_action_t action = ui_sprite_fsm_action_from_data(ctx);
	ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(action);
	ui_sprite_world_t world = ui_sprite_entity_world(entity);
	ui_sprite_camera_env_t camera = ui_sprite_camera_env_find(world);
    ui_transform_t camera_transform = ui_sprite_render_env_transform(camera->m_render);
	UI_SPRITE_EVT_CAMERA_MOVE_TO_GROUP const * evt_data = evt->data;
    ui_vector_2 world_lt = UI_VECTOR_2_ZERO;
    ui_vector_2 world_rb = UI_VECTOR_2_ZERO;
	ui_sprite_group_t target_group;
    int entity_count = 0;
    ui_vector_2 center_pos_in_screen;
    ui_vector_2 center_pos_in_world;
    ui_vector_2 target_pos;
    ui_vector_2 target_scale;

    ui_sprite_camera_updator_stop(&move->m_updator, camera);

    target_group = ui_sprite_group_find_by_name(world,  evt_data->group_name);
    if (target_group == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): camera on move to group: group %s not exist!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), evt_data->group_name);
        ui_sprite_fsm_action_sync_update(action, 0);
        return;
    }

    entity_count = ui_sprite_2d_merge_contain_rect_group(&world_lt, &world_rb, target_group);
    if (entity_count < 0) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): camera on move to group: group %s get entities fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), evt_data->group_name);
        ui_sprite_fsm_action_sync_update(action, 0);
        return;
    }
    else if (entity_count == 0) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): camera on move to group: group %s: no entity!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), evt_data->group_name);
        ui_sprite_fsm_action_sync_update(action, 0);
        return;
    }

    if (evt_data->scale == 0.0f) {
        target_scale.x = camera_transform->m_s.x;
        target_scale.y = camera_transform->m_s.y;
    }
    else {
        target_scale.x = evt_data->scale;
        target_scale.y = evt_data->scale;
    }
    
    if ((world_rb.x - world_lt.x) * target_scale.x < (evt_data->group_screen_rb.x - evt_data->group_screen_lt.x)) {
        target_scale.x = (evt_data->group_screen_rb.x - evt_data->group_screen_lt.x) / (world_rb.x - world_lt.x);
    }

    if ((world_rb.y - world_lt.y) * target_scale.y < (evt_data->group_screen_rb.y - evt_data->group_screen_lt.y)) {
        target_scale.y = (evt_data->group_screen_rb.y - evt_data->group_screen_lt.y) / (world_rb.y - world_lt.y);
    }
    
    center_pos_in_world.x = (world_lt.x + world_rb.x) / 2.0f;
    center_pos_in_world.y = (world_lt.y + world_rb.y) / 2.0f;
    center_pos_in_screen.x = (evt_data->group_screen_lt.x + evt_data->group_screen_rb.x) / 2.0f;
    center_pos_in_screen.y = (evt_data->group_screen_lt.y + evt_data->group_screen_rb.y) / 2.0f;

    target_pos =
        ui_sprite_camera_move_target_camera_pos(
            move, camera, &center_pos_in_screen, &center_pos_in_world, &target_scale);

    ui_sprite_camera_env_adj_camera_in_limit(camera, &target_pos, &target_scale);

    ui_sprite_camera_updator_set_max_speed(&move->m_updator, evt_data->max_speed);

    ui_sprite_camera_updator_set_camera(&move->m_updator, camera, target_pos, &target_scale);

    ui_sprite_fsm_action_sync_update(action, ui_sprite_camera_updator_is_runing(&move->m_updator));
}

static void ui_sprite_camera_move_on_move_to_pos(void * ctx, ui_sprite_event_t evt) {
    ui_sprite_camera_move_t move = ctx;
	ui_sprite_fsm_action_t action = ui_sprite_fsm_action_from_data(ctx);
	ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(action);
	ui_sprite_world_t world = ui_sprite_entity_world(entity);
	ui_sprite_camera_env_t camera = ui_sprite_camera_env_find(world);
    ui_transform_t camera_transform = ui_sprite_render_env_transform(camera->m_render);
	UI_SPRITE_EVT_CAMERA_MOVE_TO_POS const * evt_data = evt->data;
    ui_vector_2 target_pos_in_screen;
    ui_vector_2 target_pos_in_world;
    ui_vector_2 camera_target_pos;
    ui_vector_2 camera_target_scale;

    ui_sprite_camera_updator_stop(&move->m_updator, camera);

    if (evt_data->scale == 0.0f) {
        camera_target_scale.x = camera_transform->m_s.x;
        camera_target_scale.y = camera_transform->m_s.x;
    }
    else {
        camera_target_scale.x = evt_data->scale;
        camera_target_scale.y = evt_data->scale;
    }

    target_pos_in_world.x = evt_data->pos_in_world.x;
    target_pos_in_world.y = evt_data->pos_in_world.y;
    target_pos_in_screen.x = evt_data->pos_in_screen.x;
    target_pos_in_screen.y = evt_data->pos_in_screen.y;

    camera_target_pos =
        ui_sprite_camera_move_target_camera_pos(
            move, camera, &target_pos_in_screen, &target_pos_in_world, &camera_target_scale);

    ui_sprite_camera_env_adj_camera_in_limit(camera, &camera_target_pos, &camera_target_scale);

    ui_sprite_camera_updator_set_max_speed(&move->m_updator, evt_data->max_speed);

    ui_sprite_camera_updator_set_camera(&move->m_updator, camera, camera_target_pos, &camera_target_scale);

    ui_sprite_fsm_action_sync_update(action, ui_sprite_camera_updator_is_runing(&move->m_updator));
}

static int ui_sprite_camera_move_enter(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_camera_move_t move = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_camera_module_t module = ctx;

	if (ui_sprite_fsm_action_add_event_handler(
            fsm_action, ui_sprite_event_scope_self, 
            "ui_sprite_evt_camera_move_to_entity", ui_sprite_camera_move_on_move_to_entity, move) != 0
        || ui_sprite_fsm_action_add_event_handler(
            fsm_action, ui_sprite_event_scope_self, 
            "ui_sprite_evt_camera_move_to_group", ui_sprite_camera_move_on_move_to_group, move) != 0
        || ui_sprite_fsm_action_add_event_handler(
            fsm_action, ui_sprite_event_scope_self, 
            "ui_sprite_evt_camera_move_to_pos", ui_sprite_camera_move_on_move_to_pos, move) != 0
        )
	{    
		CPE_ERROR(module->m_em, "camera move enter: add eventer handler fail!");
		return -1;
	}

    return 0;
}

static void ui_sprite_camera_move_exit(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_camera_move_t move = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_camera_module_t module = ctx;
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_world_t world = ui_sprite_entity_world(entity);
    ui_sprite_camera_env_t camera = ui_sprite_camera_env_find(world);

    if (camera == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): camera move exit: no camera!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return;
    }

    ui_sprite_camera_updator_stop(&move->m_updator, camera);
}

static void ui_sprite_camera_move_update(ui_sprite_fsm_action_t fsm_action, void * ctx, float delta) {
    ui_sprite_camera_move_t move = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_camera_module_t module = ctx;
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_world_t world = ui_sprite_entity_world(entity);
    ui_sprite_camera_env_t camera = ui_sprite_camera_env_find(world);

    if (camera == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): camera move: no camera!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        ui_sprite_fsm_action_stop_update(fsm_action);
        return;
    }

    ui_sprite_camera_updator_update(&move->m_updator, camera, delta);

    if (!ui_sprite_camera_updator_is_runing(&move->m_updator)) {
        ui_sprite_fsm_action_stop_update(fsm_action);
    }
}

static int ui_sprite_camera_move_init(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_camera_move_t move = ui_sprite_fsm_action_data(fsm_action);

    bzero(move, sizeof(*move));

    move->m_module = ctx;

    return 0;
}

static void ui_sprite_camera_move_clear(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_camera_move_t move = ui_sprite_fsm_action_data(fsm_action);

    assert(move->m_updator.m_curent_op_id == 0);
}

static int ui_sprite_camera_move_copy(ui_sprite_fsm_action_t to, ui_sprite_fsm_action_t from, void * ctx) {
    if (ui_sprite_camera_move_init(to, ctx)) return -1;
    return 0;
}

static ui_sprite_fsm_action_t ui_sprite_camera_move_load(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg) {
    ui_sprite_camera_module_t module = ctx;
    ui_sprite_camera_move_t camera_camera_move = ui_sprite_camera_move_create(fsm_state, name);
    const char * decorator;

    if (camera_camera_move == NULL) {
        CPE_ERROR(module->m_em, "%s: create camera_camera_move action: create fail!", ui_sprite_camera_module_name(module));
        return NULL;
    }

    if ((decorator = cfg_get_string(cfg, "decorator", NULL))) {
        if (ui_sprite_camera_move_set_decorator(camera_camera_move, decorator) != 0) {
            CPE_ERROR(module->m_em, "%s: create camera_camera_move action: create fail!", ui_sprite_camera_module_name(module));
            ui_sprite_camera_move_free(camera_camera_move);
            return NULL;
        }
    }

    return ui_sprite_fsm_action_from_data(camera_camera_move);
}

int ui_sprite_camera_move_regist(ui_sprite_camera_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_create(module->m_fsm_module, UI_SPRITE_CAMERA_MOVE_NAME, sizeof(struct ui_sprite_camera_move));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: camera camera move register: meta create fail",
            ui_sprite_camera_module_name(module));
        return -1;
    }

    ui_sprite_fsm_action_meta_set_enter_fun(meta, ui_sprite_camera_move_enter, module);
    ui_sprite_fsm_action_meta_set_exit_fun(meta, ui_sprite_camera_move_exit, module);
    ui_sprite_fsm_action_meta_set_init_fun(meta, ui_sprite_camera_move_init, module);
    ui_sprite_fsm_action_meta_set_copy_fun(meta, ui_sprite_camera_move_copy, module);
    ui_sprite_fsm_action_meta_set_free_fun(meta, ui_sprite_camera_move_clear, module);
    ui_sprite_fsm_action_meta_set_update_fun(meta, ui_sprite_camera_move_update, module);

    if (module->m_loader) {
        if (ui_sprite_cfg_loader_add_action_loader(module->m_loader, UI_SPRITE_CAMERA_MOVE_NAME, ui_sprite_camera_move_load, module) != 0) {
            ui_sprite_fsm_action_meta_free(meta);
            return -1;
        }
    }
    
    return 0;
}

void ui_sprite_camera_move_unregist(ui_sprite_camera_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_find(module->m_fsm_module, UI_SPRITE_CAMERA_MOVE_NAME);
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: camera camera move unregister: meta not exist",
            ui_sprite_camera_module_name(module));
        return;
    }

    ui_sprite_fsm_action_meta_free(meta);

    if (module->m_loader) {
        ui_sprite_cfg_loader_remove_action_loader(module->m_loader, UI_SPRITE_CAMERA_MOVE_NAME);
    }
}

const char * UI_SPRITE_CAMERA_MOVE_NAME = "camera-move";

