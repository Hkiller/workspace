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
#include "ui/sprite_2d/ui_sprite_2d_transform.h"
#include "ui/sprite_2d/ui_sprite_2d_utils.h"
#include "ui_sprite_camera_follow_i.h"
#include "ui_sprite_camera_utils.h"
#include "protocol/ui/sprite_camera/ui_sprite_camera_evt.h"

ui_sprite_camera_follow_t ui_sprite_camera_follow_create(ui_sprite_fsm_state_t fsm_state, const char * name) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_create(fsm_state, name, UI_SPRITE_CAMERA_FOLLOW_NAME);
    return fsm_action ? ui_sprite_fsm_action_data(fsm_action) : NULL;
}

void ui_sprite_camera_follow_free(ui_sprite_camera_follow_t follow) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_from_data(follow);
    ui_sprite_fsm_action_free(fsm_action);
}

int ui_sprite_camera_follow_set_decorator(ui_sprite_camera_follow_t follow, const char * decorator) {
    return ui_percent_decorator_setup(&follow->m_updator.m_decorator, decorator, follow->m_module->m_em);
}

static void ui_sprite_camera_follow_calc_target_camera_by_x(
    ui_sprite_camera_follow_t follow, ui_sprite_camera_env_t camera, ui_vector_2_t screen_size, ui_sprite_entity_t entity, 
    ui_vector_2_t target_scale, ui_vector_2 follow_world_pos)
{
    ui_vector_2 screen_pt;
    float check_delta;
    uint8_t check_count;
    ui_vector_2 scale_min;
    ui_vector_2 scale_max;

    screen_pt.x = follow->m_follow_pos_in_screen.x;

    screen_pt.y = ui_sprite_camera_env_screen_x2y_lock_x(camera, screen_pt.x, follow_world_pos, target_scale);

    if (ui_sprite_2d_pt_in_rect(screen_pt, &follow->m_screen_rect)) {
        return;
    }

    scale_min = scale_max = *target_scale;
    for(check_count = 0; ; check_count++) {
        assert(check_count < 100);

        scale_max.x *= 1.1f;

        screen_pt.y = ui_sprite_camera_env_screen_x2y_lock_x(camera, screen_pt.x, follow_world_pos, &scale_max);
            
        if (ui_sprite_2d_pt_in_rect(screen_pt, &follow->m_screen_rect)) {
            break;
        }
        else {
            scale_min = scale_max;
        }
    }

    check_delta = 0.49f / screen_size->x;
    do {
        assert(++check_count < 100);

        target_scale->x = (scale_min.x + scale_max.x) / 2.0f;

        screen_pt.y = ui_sprite_camera_env_screen_x2y_lock_x(camera, screen_pt.x, follow_world_pos, target_scale);

        if (ui_sprite_2d_pt_in_rect(screen_pt, &follow->m_screen_rect)) {
            scale_max.x = target_scale->x;
        }
        else {
            scale_min.x = target_scale->x;
        }

        assert(scale_min.x < scale_max.x);
    } while((scale_max.x - scale_min.x) > check_delta);
}

static void ui_sprite_camera_follow_calc_target_camera(
    ui_sprite_camera_follow_t follow, ui_sprite_camera_env_t camera, ui_sprite_entity_t entity, 
    ui_vector_2 * target_camera_pos, ui_vector_2_t target_scale, ui_vector_2 follow_world_pos)
{
    ui_vector_2_t camera_size = ui_sprite_render_env_size(camera->m_render);
    ui_transform_t camera_trans = ui_sprite_render_env_transform(camera->m_render);
    
    if (follow->m_best_scale) {
        target_scale->x = follow->m_best_scale;
        target_scale->y = follow->m_best_scale;
    }
    else {
        target_scale->x = camera_trans->m_s.x;
        target_scale->y = camera_trans->m_s.y;
    }

    /*如果没有限制，则直接计算位置返回 */
    if (!ui_sprite_camera_env_have_limit(camera)) {
        *target_camera_pos = ui_sprite_camera_calc_pos(camera, follow_world_pos, follow->m_follow_pos_in_screen, target_scale);
        return;
    }

    /*如果没有锁定点，不需要关注自动缩放，只需要限制在范围内就可以了 */
    switch(camera->m_trace_type) {
    case ui_sprite_camera_trace_none: {
        float max_scale_in_limit = ui_sprite_camera_max_scale_from_point(camera, follow_world_pos, follow->m_follow_pos_in_screen);
        if (max_scale_in_limit > target_scale->x) target_scale->x = max_scale_in_limit;
        if (max_scale_in_limit > target_scale->y) target_scale->y = max_scale_in_limit;
        *target_camera_pos = ui_sprite_camera_calc_pos(camera, follow_world_pos, follow->m_follow_pos_in_screen, target_scale);
        return;
    }
    case ui_sprite_camera_trace_by_x: {
        ui_sprite_camera_follow_calc_target_camera_by_x(follow, camera, camera_size, entity, target_scale, follow_world_pos);
        *target_camera_pos = ui_sprite_camera_calc_pos(camera, follow_world_pos, follow->m_follow_pos_in_screen, target_scale);
        return;
    }
    case ui_sprite_camera_trace_by_y: {
        assert(0);
        return;
    }
    default:
        assert(0);
        *target_camera_pos = ui_sprite_camera_calc_pos(camera, follow_world_pos, follow->m_follow_pos_in_screen, target_scale);
        return;
    }
}

static void ui_sprite_camera_follow_on_follow_entity(void * ctx, ui_sprite_event_t evt) {
    ui_sprite_camera_follow_t follow = ctx;
    ui_sprite_camera_module_t module = follow->m_module;
	ui_sprite_fsm_action_t action = ui_sprite_fsm_action_from_data(ctx);
	ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(action);
	ui_sprite_world_t world = ui_sprite_entity_world(entity);
	ui_sprite_camera_env_t camera = ui_sprite_camera_env_find(world);
	UI_SPRITE_EVT_CAMERA_FOLLOW_ENTITY const * evt_data = evt->data;

    ui_sprite_camera_updator_stop(&follow->m_updator, camera);

    if (evt_data->screen_lt.x < 0.0f || evt_data->screen_lt.x > 1.0f
        || evt_data->screen_lt.y < 0.0f || evt_data->screen_lt.y > 1.0f
        || evt_data->screen_rb.x < 0.0f || evt_data->screen_rb.x > 1.0f
        || evt_data->screen_rb.y < 0.0f || evt_data->screen_rb.y > 1.0f
        || evt_data->screen_lt.x > evt_data->screen_rb.x
        || evt_data->screen_lt.y > evt_data->screen_rb.y)
    {
        CPE_ERROR(
            module->m_em, "entity %d(%s): camera on follow entity: screen rect (%f,%f) - (%f,%f) error!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), 
            evt_data->screen_lt.x, evt_data->screen_lt.y, evt_data->screen_rb.x, evt_data->screen_rb.y);
        ui_sprite_fsm_action_sync_update(action, 0);
        return;
    }

    follow->m_follow_entity_pos = ui_sprite_2d_transform_pos_policy_from_str(evt_data->pos_of_entity);
    if (follow->m_follow_entity_pos == 0) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): camera on follow to entity: pos of entity %s is unknown!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), evt_data->pos_of_entity);
        ui_sprite_fsm_action_sync_update(action, 0);
        return;
    }
    follow->m_follow_entity_id = evt_data->entity_id;
    cpe_str_dup(follow->m_follow_entity_name, sizeof(follow->m_follow_entity_name), evt_data->entity_name);

    if (evt_data->screen_lt.x == evt_data->screen_rb.x || evt_data->screen_lt.y == evt_data->screen_rb.y) {
        follow->m_screen_rect.lt.x = 0.0f;
        follow->m_screen_rect.lt.y = 0.0f;
        follow->m_screen_rect.rb.x = 1.0f;
        follow->m_screen_rect.rb.y = 1.0f;
    }
    else {
        follow->m_screen_rect.lt.x = evt_data->screen_lt.x;
        follow->m_screen_rect.lt.y = evt_data->screen_lt.y;
        follow->m_screen_rect.rb.x = evt_data->screen_rb.x;
        follow->m_screen_rect.rb.y = evt_data->screen_rb.y;
    }

    follow->m_follow_pos_in_screen.x = evt_data->pos_in_screen.x;
    follow->m_follow_pos_in_screen.y = evt_data->pos_in_screen.y;

    follow->m_best_scale = evt_data->best_scale;

    ui_sprite_camera_updator_set_max_speed(&follow->m_updator, evt_data->max_speed);

    ui_sprite_fsm_action_sync_update(action, 1);
}

static int ui_sprite_camera_follow_enter(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_camera_follow_t follow = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_camera_module_t module = ctx;

	if (ui_sprite_fsm_action_add_event_handler(
            fsm_action, ui_sprite_event_scope_self, 
            "ui_sprite_evt_camera_follow_entity", ui_sprite_camera_follow_on_follow_entity, follow) != 0
        )
	{    
		CPE_ERROR(module->m_em, "camera follow entity: add eventer handler fail!");
		return -1;
	}
	
    return 0;
}

static void ui_sprite_camera_follow_exit(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_camera_follow_t follow = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_camera_module_t module = ctx;
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_world_t world = ui_sprite_entity_world(entity);
    ui_sprite_camera_env_t camera = ui_sprite_camera_env_find(world);

    ui_sprite_camera_updator_stop(&follow->m_updator, camera);

    if (camera == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): camera follow entity: exit: no camera!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return;
    }
}

static void ui_sprite_camera_follow_update(ui_sprite_fsm_action_t fsm_action, void * ctx, float delta) {
    ui_sprite_camera_follow_t follow = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_camera_module_t module = ctx;
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_world_t world = ui_sprite_entity_world(entity);
    ui_sprite_camera_env_t camera = ui_sprite_camera_env_find(world);
    ui_vector_2 follow_world_pos;
    ui_vector_2 target_camera_pos;
    ui_vector_2 target_scale;

    if (camera == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): camera follow entity: update: no camera!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        goto FOLLOW_STOP_UPDATE;
    }

    if (ui_sprite_camera_env_pos_of_entity(
            &follow_world_pos, world,
            follow->m_follow_entity_id, follow->m_follow_entity_name, follow->m_follow_entity_pos)
        != 0)
    {
        CPE_ERROR(
            module->m_em, "entity %d(%s): camera follow entity: get pos of entity %d(%s) fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), follow->m_follow_entity_id, follow->m_follow_entity_name);
        goto FOLLOW_STOP_UPDATE;
    }

    ui_sprite_camera_follow_calc_target_camera(follow, camera, entity, &target_camera_pos, &target_scale, follow_world_pos);

    ui_sprite_camera_updator_set_camera(&follow->m_updator, camera, target_camera_pos, &target_scale);

    return;

FOLLOW_STOP_UPDATE:
    ui_sprite_camera_updator_stop(&follow->m_updator, camera);
    ui_sprite_fsm_action_stop_update(fsm_action);
}

static int ui_sprite_camera_follow_init(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_camera_follow_t follow = ui_sprite_fsm_action_data(fsm_action);

    bzero(follow, sizeof(*follow));

    follow->m_module = ctx;

    return 0;
}

static void ui_sprite_camera_follow_clear(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_camera_follow_t follow = ui_sprite_fsm_action_data(fsm_action);

    assert(follow->m_updator.m_curent_op_id == 0);
}

static int ui_sprite_camera_follow_copy(ui_sprite_fsm_action_t to, ui_sprite_fsm_action_t from, void * ctx) {
    if (ui_sprite_camera_follow_init(to, ctx)) return -1;
    return 0;
}

ui_sprite_fsm_action_t ui_sprite_camera_follow_load(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg) {
    ui_sprite_camera_module_t module = ctx;
    ui_sprite_camera_follow_t camera_camera_follow = ui_sprite_camera_follow_create(fsm_state, name);
    const char * decorator;

    if (camera_camera_follow == NULL) {
        CPE_ERROR(module->m_em, "%s: create camera_camera_follow action: create fail!", ui_sprite_camera_module_name(module));
        return NULL;
    }

    if ((decorator = cfg_get_string(cfg, "decorator", NULL))) {
        if (ui_sprite_camera_follow_set_decorator(camera_camera_follow, decorator) != 0) {
            CPE_ERROR(module->m_em, "%s: create camera_camera_follow action: create fail!", ui_sprite_camera_module_name(module));
            ui_sprite_camera_follow_free(camera_camera_follow);
            return NULL;
        }
    }

    return ui_sprite_fsm_action_from_data(camera_camera_follow);
}

int ui_sprite_camera_follow_regist(ui_sprite_camera_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_create(module->m_fsm_module, UI_SPRITE_CAMERA_FOLLOW_NAME, sizeof(struct ui_sprite_camera_follow));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: camera camera follow register: meta create fail",
            ui_sprite_camera_module_name(module));
        return -1;
    }

    ui_sprite_fsm_action_meta_set_enter_fun(meta, ui_sprite_camera_follow_enter, module);
    ui_sprite_fsm_action_meta_set_exit_fun(meta, ui_sprite_camera_follow_exit, module);
    ui_sprite_fsm_action_meta_set_init_fun(meta, ui_sprite_camera_follow_init, module);
    ui_sprite_fsm_action_meta_set_copy_fun(meta, ui_sprite_camera_follow_copy, module);
    ui_sprite_fsm_action_meta_set_free_fun(meta, ui_sprite_camera_follow_clear, module);
    ui_sprite_fsm_action_meta_set_update_fun(meta, ui_sprite_camera_follow_update, module);

    if (module->m_loader) {
        if (ui_sprite_cfg_loader_add_action_loader(module->m_loader, UI_SPRITE_CAMERA_FOLLOW_NAME, ui_sprite_camera_follow_load, module) != 0) {
            ui_sprite_fsm_action_meta_free(meta);
            return -1;
        }
    }
    
    return 0;
}

void ui_sprite_camera_follow_unregist(ui_sprite_camera_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_find(module->m_fsm_module, UI_SPRITE_CAMERA_FOLLOW_NAME);
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: camera camera follow unregister: meta not exist",
            ui_sprite_camera_module_name(module));
        return;
    }

    ui_sprite_fsm_action_meta_free(meta);

    if (module->m_loader) {
        ui_sprite_cfg_loader_remove_action_loader(module->m_loader, UI_SPRITE_CAMERA_FOLLOW_NAME);
    }
}

const char * UI_SPRITE_CAMERA_FOLLOW_NAME = "camera-follow";

