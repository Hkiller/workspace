#include "cpe/utils/math_ex.h"
#include "cpe/cfg/cfg_read.h"
#include "render/utils/ui_transform.h"
#include "ui/sprite/ui_sprite_world.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_meta.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui/sprite_cfg/ui_sprite_cfg_loader.h"
#include "ui/sprite_2d/ui_sprite_2d_transform.h"
#include "ui_sprite_camera_shake_i.h"
#include "ui_sprite_camera_env_i.h"
#include "protocol/ui/sprite_camera/ui_sprite_camera_evt.h"

ui_sprite_camera_shake_t ui_sprite_camera_shake_create(ui_sprite_fsm_state_t fsm_state, const char * name) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_create(fsm_state, name, UI_SPRITE_CAMERA_SHAKE_NAME);
    return fsm_action ? ui_sprite_fsm_action_data(fsm_action) : NULL;
}

void ui_sprite_camera_shake_free(ui_sprite_camera_shake_t shake) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_from_data(shake);
    ui_sprite_fsm_action_free(fsm_action);
}

static void ui_sprite_camera_shake_on_shake_to(void * ctx, ui_sprite_event_t evt) {
	ui_sprite_camera_shake_t shake = ctx;
	ui_sprite_fsm_action_t action = ui_sprite_fsm_action_from_data(ctx);
	ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(action);
	ui_sprite_world_t world = ui_sprite_entity_world(entity);  
	ui_sprite_camera_env_t camera = ui_sprite_camera_env_find(world);
    ui_transform_t camera_transform = ui_sprite_render_env_transform(camera->m_render);
	UI_SPRITE_EVT_CAMERA_SHAKE_TO const * evt_data = evt->data;

	shake->m_num = evt_data->num;
	if(shake->m_num == 0
       || (evt_data->amplitude.x ==0 && evt_data->amplitude.y == 0)
       || evt_data->duration == 0.0f)
    {
		return;
	}
    
	shake->m_once_duration = evt_data->duration / shake->m_num;
	shake->m_speed.x = evt_data->amplitude.x / (shake->m_once_duration / 4.0f);
	shake->m_speed.y = evt_data->amplitude.y / (shake->m_once_duration / 4.0f);
	shake->m_work_duration = 0.0f;
	shake->m_amplitude_count = 0;
    ui_transform_get_pos_2(camera_transform, &shake->m_camera_pos);
	ui_sprite_fsm_action_sync_update(ui_sprite_fsm_action_from_data(shake), 1);
}

static int ui_sprite_camera_shake_enter(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_camera_shake_t shake = ui_sprite_fsm_action_data(fsm_action); 
    ui_sprite_camera_module_t module = ctx;

	if (ui_sprite_fsm_action_add_event_handler(
		fsm_action, ui_sprite_event_scope_self, 
		"ui_sprite_evt_camera_shake_to", ui_sprite_camera_shake_on_shake_to, shake) != 0)
	{    
		CPE_ERROR(module->m_em, "camera shake enter: add eventer handler fail!");
		return -1;
	}
    return 0;
}

static void ui_sprite_camera_shake_exit(ui_sprite_fsm_action_t fsm_action, void * ctx) {

}

static int ui_sprite_camera_shake_init(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_camera_shake_t shake = ui_sprite_fsm_action_data(fsm_action);
    shake->m_module = ctx;
	shake->m_speed.x = 0.0f;
	shake->m_speed.y = 0.0f;
	shake->m_num = 0;
	shake->m_once_duration= 0.0f;
	shake->m_work_duration= 0.0f;
	shake->m_camera_pos.x = 0.0f;
	shake->m_camera_pos.y = 0.0f;
	shake->m_amplitude_count =0;
    return 0;
}

static void ui_sprite_camera_shake_clear(ui_sprite_fsm_action_t fsm_action, void * ctx) {
}

static int ui_sprite_camera_shake_copy(ui_sprite_fsm_action_t to, ui_sprite_fsm_action_t from, void * ctx) {
	ui_sprite_camera_shake_t shake_to = ui_sprite_fsm_action_data(to);
	ui_sprite_camera_shake_t shake_from = ui_sprite_fsm_action_data(from);

    if (ui_sprite_camera_shake_init(to, ctx)) return -1;

	shake_to->m_speed.x = shake_from->m_speed.x;
	shake_to->m_speed.y = shake_from->m_speed.y;
	shake_to->m_num = shake_from->m_num;
	shake_to->m_once_duration= shake_from->m_once_duration;
	shake_to->m_work_duration= shake_from->m_work_duration;
	shake_to->m_amplitude_count = shake_from->m_amplitude_count;

    return 0;
}

static void ui_sprite_camera_shake_update(ui_sprite_fsm_action_t fsm_action, void * ctx, float delta) {
	ui_sprite_camera_shake_t shake = ui_sprite_fsm_action_data(fsm_action);
	ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
	ui_sprite_world_t world = ui_sprite_entity_world(entity);
	ui_sprite_camera_env_t camera = ui_sprite_camera_env_find(world);
    ui_transform_t camera_transform = ui_sprite_render_env_transform(camera->m_render);
	ui_vector_2 camera_pos;
    ui_transform target_transform;
    
    ui_transform_get_pos_2(camera_transform, &camera_pos);
    cpe_assert_float_sane(camera_pos.x);
    cpe_assert_float_sane(camera_pos.y);

	if(shake->m_speed.x != 0.0f){
		if(shake->m_amplitude_count % 2 == 0)
			camera_pos.x += shake->m_speed.x * delta;
		else 
			camera_pos.x -= shake->m_speed.x * delta;
	}

	if(shake->m_speed.y != 0.0f){
		if(shake->m_amplitude_count % 2 == 0)
			camera_pos.y += shake->m_speed.y * delta;
		else 
			camera_pos.y -= shake->m_speed.y * delta;
	}

    target_transform = *camera_transform;
    ui_transform_set_pos_2(&target_transform, &camera_pos);
	ui_sprite_render_env_set_transform(camera->m_render, &target_transform);
    
	shake->m_work_duration += delta;
	if(shake->m_work_duration >= shake->m_once_duration / 4.0f){
		shake->m_work_duration = 0.0f;
		shake->m_amplitude_count++;
	}
	//CPE_ERROR( module->m_em, "shake ==== %d", shake->m_num);
	if(shake->m_amplitude_count >= 4){
		shake->m_num --;
		shake->m_amplitude_count = 0.0f;
		if(shake->m_num == 0){
            target_transform = *camera_transform;
            ui_transform_set_pos_2(&target_transform, &shake->m_camera_pos);
            ui_sprite_render_env_set_transform(camera->m_render, &target_transform);

			ui_sprite_fsm_action_stop_update(fsm_action);
		}
	}
}

static ui_sprite_fsm_action_t ui_sprite_camera_shake_load(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg) {
    ui_sprite_camera_module_t module = ctx;
    ui_sprite_camera_shake_t camera_camera_shake = ui_sprite_camera_shake_create(fsm_state, name);

    if (camera_camera_shake == NULL) {
        CPE_ERROR(module->m_em, "%s: create camera_camera_shake action: create fail!", ui_sprite_camera_module_name(module));
        return NULL;
    }

    return ui_sprite_fsm_action_from_data(camera_camera_shake);
}

int ui_sprite_camera_shake_regist(ui_sprite_camera_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_create(module->m_fsm_module, UI_SPRITE_CAMERA_SHAKE_NAME, sizeof(struct ui_sprite_camera_shake));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: camera camera shake register: meta create fail",
            ui_sprite_camera_module_name(module));
        return -1;
    }

    ui_sprite_fsm_action_meta_set_enter_fun(meta, ui_sprite_camera_shake_enter, module);
    ui_sprite_fsm_action_meta_set_exit_fun(meta, ui_sprite_camera_shake_exit, module);
    ui_sprite_fsm_action_meta_set_init_fun(meta, ui_sprite_camera_shake_init, module);
    ui_sprite_fsm_action_meta_set_copy_fun(meta, ui_sprite_camera_shake_copy, module);
    ui_sprite_fsm_action_meta_set_free_fun(meta, ui_sprite_camera_shake_clear, module);
	ui_sprite_fsm_action_meta_set_update_fun(meta, ui_sprite_camera_shake_update, module);

    if (module->m_loader) {
        if (ui_sprite_cfg_loader_add_action_loader(module->m_loader, UI_SPRITE_CAMERA_SHAKE_NAME, ui_sprite_camera_shake_load, module) != 0) {
            ui_sprite_fsm_action_meta_free(meta);
            return -1;
        }
    }
    
    return 0;
}

void ui_sprite_camera_shake_unregist(ui_sprite_camera_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_find(module->m_fsm_module, UI_SPRITE_CAMERA_SHAKE_NAME);
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: camera camera shake unregister: meta not exist",
            ui_sprite_camera_module_name(module));
        return;
    }

    ui_sprite_fsm_action_meta_free(meta);

    if (module->m_loader) {
        ui_sprite_cfg_loader_remove_action_loader(module->m_loader, UI_SPRITE_CAMERA_SHAKE_NAME);
    }
}

const char * UI_SPRITE_CAMERA_SHAKE_NAME = "camera-shake";

