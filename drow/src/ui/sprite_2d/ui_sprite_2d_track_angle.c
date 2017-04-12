#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/math_ex.h"
#include "cpe/cfg/cfg_read.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_meta.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui/sprite_cfg/ui_sprite_cfg_loader.h"
#include "ui/sprite_2d/ui_sprite_2d_track_angle.h"
#include "ui_sprite_2d_track_angle_i.h"
#include "ui_sprite_2d_module_i.h"
#include "ui_sprite_2d_transform_i.h"
#include "protocol/ui/sprite_2d/ui_sprite_2d_evt.h"

ui_sprite_2d_track_angle_t
ui_sprite_2d_track_angle_create(ui_sprite_fsm_state_t fsm_state, const char * name) {
    ui_sprite_fsm_action_t fsm_action;
    fsm_action = ui_sprite_fsm_action_create(fsm_state, name, UI_SPRITE_2D_TRACK_ANGLE_NAME);
    return fsm_action ? ui_sprite_fsm_action_data(fsm_action) : NULL;
}

void ui_sprite_2d_track_angle_free(ui_sprite_2d_track_angle_t show_anim) {
    ui_sprite_fsm_action_free(ui_sprite_fsm_action_from_data(show_anim));
}

int ui_sprite_2d_track_angle_enter(ui_sprite_fsm_action_t fsm_action, void * ctx) {
	ui_sprite_2d_track_angle_t track_angle = ui_sprite_fsm_action_data(fsm_action);

    track_angle->m_total_time = 0.0f;
    track_angle->m_moving_poses_wp = track_angle->m_moving_poses;
    track_angle->m_moving_poses_rp = track_angle->m_moving_poses;
    
    ui_sprite_fsm_action_start_update(fsm_action);

    return 0;
}

void ui_sprite_2d_track_angle_update(ui_sprite_fsm_action_t fsm_action, void * ctx, float delta) {
    ui_sprite_2d_module_t module = ctx;
	ui_sprite_2d_track_angle_t track_angle = ui_sprite_fsm_action_data(fsm_action);    
	ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
	ui_sprite_2d_transform_t transform = ui_sprite_2d_transform_find(entity);
    struct ui_sprite_2d_track_angle_pos * end;

    if (transform == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): track angle: update: no transform!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        ui_sprite_fsm_action_stop_update(fsm_action);
        return;
    }

    end = track_angle->m_moving_poses + CPE_ARRAY_SIZE(track_angle->m_moving_poses);
    assert(track_angle->m_moving_poses_wp >= track_angle->m_moving_poses && track_angle->m_moving_poses_wp < end);
    assert(track_angle->m_moving_poses_rp >= track_angle->m_moving_poses && track_angle->m_moving_poses_rp < end);
            
    track_angle->m_moving_poses_wp->m_pos.x = transform->m_data.transform.pos.x;
    track_angle->m_moving_poses_wp->m_pos.y = transform->m_data.transform.pos.y;
    track_angle->m_moving_poses_wp->m_delta = delta;

    track_angle->m_total_time += delta;

    if (track_angle->m_total_time > track_angle->m_check_span
        &&
        (fabs(track_angle->m_moving_poses_rp->m_pos.x - track_angle->m_moving_poses_wp->m_pos.x) > 1.0f
         || fabs(track_angle->m_moving_poses_rp->m_pos.y - track_angle->m_moving_poses_wp->m_pos.y) > 1.0f)
        )
    {
        float angle =
            cpe_math_angle(
                track_angle->m_moving_poses_rp->m_pos.x, track_angle->m_moving_poses_rp->m_pos.y,
                track_angle->m_moving_poses_wp->m_pos.x, track_angle->m_moving_poses_wp->m_pos.y);
        ui_sprite_2d_transform_set_angle(transform, angle);
        /* printf( */
        /*     "xxxxxxxxxxxxxx: update angle %f, (%f,%f) ==> (%f,%f)\n", angle, */
        /*     track_angle->m_moving_poses_rp->m_pos.x, track_angle->m_moving_poses_rp->m_pos.y, */
        /*     track_angle->m_moving_poses_wp->m_pos.x, track_angle->m_moving_poses_wp->m_pos.y); */
    }
        
    track_angle->m_moving_poses_wp++;
    if (track_angle->m_moving_poses_wp >= end) track_angle->m_moving_poses_wp = track_angle->m_moving_poses;

    if (track_angle->m_moving_poses_wp == track_angle->m_moving_poses_rp) {
        track_angle->m_total_time -= track_angle->m_moving_poses_rp->m_delta;
        track_angle->m_moving_poses_rp++;
        if (track_angle->m_moving_poses_rp >= end) track_angle->m_moving_poses_rp = track_angle->m_moving_poses;
    }

    while(track_angle->m_total_time > track_angle->m_check_span
          && track_angle->m_moving_poses_rp != track_angle->m_moving_poses_wp)
    {
        track_angle->m_total_time -= track_angle->m_moving_poses_rp->m_delta;
        track_angle->m_moving_poses_rp++;
        if (track_angle->m_moving_poses_rp >= end) track_angle->m_moving_poses_rp = track_angle->m_moving_poses;
    }
}

void ui_sprite_2d_track_angle_exit(ui_sprite_fsm_action_t fsm_action, void * ctx) {
}

int ui_sprite_2d_track_angle_init(ui_sprite_fsm_action_t fsm_action, void * ctx) {
	ui_sprite_2d_track_angle_t track_angle = ui_sprite_fsm_action_data(fsm_action);

	bzero(track_angle, sizeof(*track_angle));
	track_angle->m_module = ctx;
    track_angle->m_check_span = 0.1f;

	return 0;
}

int ui_sprite_2d_track_angle_copy(ui_sprite_fsm_action_t to, ui_sprite_fsm_action_t from, void * ctx) {
    ui_sprite_2d_track_angle_init(to, ctx);
    return 0;
}

void ui_sprite_2d_track_angle_clear(ui_sprite_fsm_action_t fsm_action, void * ctx) {
}

ui_sprite_fsm_action_t ui_sprite_2d_track_angle_load(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg) {
	ui_sprite_2d_module_t module = ctx;
	ui_sprite_2d_track_angle_t p2d_track_angle = ui_sprite_2d_track_angle_create(fsm_state, name);

	if (p2d_track_angle == NULL) {
		CPE_ERROR(module->m_em, "%s: create anim_2d_track_angle action: create fail!", ui_sprite_2d_module_name(module));
		return NULL;
	}

	return ui_sprite_fsm_action_from_data(p2d_track_angle);
}

int ui_sprite_2d_track_angle_regist(ui_sprite_2d_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_create(
        module->m_fsm_module, UI_SPRITE_2D_TRACK_ANGLE_NAME, sizeof(struct ui_sprite_2d_track_angle));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: %s register: meta create fail",
            ui_sprite_2d_module_name(module), UI_SPRITE_2D_TRACK_ANGLE_NAME);
        return -1;
    }

    ui_sprite_fsm_action_meta_set_init_fun(meta, ui_sprite_2d_track_angle_init, module);
    ui_sprite_fsm_action_meta_set_copy_fun(meta, ui_sprite_2d_track_angle_copy, module);
    ui_sprite_fsm_action_meta_set_enter_fun(meta, ui_sprite_2d_track_angle_enter, module);
    ui_sprite_fsm_action_meta_set_exit_fun(meta, ui_sprite_2d_track_angle_exit, module);
    ui_sprite_fsm_action_meta_set_free_fun(meta, ui_sprite_2d_track_angle_clear, module);
	ui_sprite_fsm_action_meta_set_update_fun(meta, ui_sprite_2d_track_angle_update, module);

    if (module->m_loader) {
        if (ui_sprite_cfg_loader_add_action_loader(module->m_loader, UI_SPRITE_2D_TRACK_ANGLE_NAME, ui_sprite_2d_track_angle_load, module) != 0) {
            ui_sprite_fsm_action_meta_free(meta);
            return -1;
        }
    }
    
    return 0;
}

void ui_sprite_2d_track_angle_unregist(ui_sprite_2d_module_t module) {
	ui_sprite_fsm_action_meta_t meta;

	meta = ui_sprite_fsm_action_meta_find(module->m_fsm_module, UI_SPRITE_2D_TRACK_ANGLE_NAME);
	if (meta == NULL) {
		CPE_ERROR(
			module->m_em, "%s: %s unregister: meta not exist",
			ui_sprite_2d_module_name(module), UI_SPRITE_2D_TRACK_ANGLE_NAME);
		return;
	}

	ui_sprite_fsm_action_meta_free(meta);

    if (module->m_loader) {
        ui_sprite_cfg_loader_remove_action_loader(module->m_loader, UI_SPRITE_2D_TRACK_ANGLE_NAME);
    }
}

const char * UI_SPRITE_2D_TRACK_ANGLE_NAME = "2d-track-angle";
