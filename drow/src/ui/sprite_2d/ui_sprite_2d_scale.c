#include "cpe/pal/pal_strings.h"
#include "cpe/utils/string_utils.h"
#include "cpe/utils/math_ex.h"
#include "cpe/cfg/cfg_read.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_meta.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_calc.h"
#include "ui/sprite_cfg/ui_sprite_cfg_loader.h"
#include "ui/sprite_2d/ui_sprite_2d_scale.h"
#include "ui/sprite_2d/ui_sprite_2d_transform.h"
#include "ui_sprite_2d_scale_i.h"

ui_sprite_2d_scale_t
ui_sprite_2d_scale_create(ui_sprite_fsm_state_t fsm_state, const char * name) {
    ui_sprite_fsm_action_t fsm_action;
    fsm_action = ui_sprite_fsm_action_create(fsm_state, name, UI_SPRITE_2D_SCALE_NAME);
    return fsm_action ? ui_sprite_fsm_action_data(fsm_action) : NULL;
}

void ui_sprite_2d_scale_free(ui_sprite_2d_scale_t scale) {
    ui_sprite_fsm_action_free(ui_sprite_fsm_action_from_data(scale));
}

int ui_sprite_2d_scale_set_decorator(ui_sprite_2d_scale_t scale, const char * decorator) {
    return ui_percent_decorator_setup(&scale->m_cfg_decorator, decorator, scale->m_module->m_em);
}

int ui_sprite_2d_scale_set_target_scale_x(ui_sprite_2d_scale_t scale, const char * target_scale_x) {
    if (scale->m_cfg_target_scale_x) {
        mem_free(scale->m_module->m_alloc, scale->m_cfg_target_scale_x);
    }

    if (target_scale_x) {
        scale->m_cfg_target_scale_x = cpe_str_mem_dup_trim(scale->m_module->m_alloc, target_scale_x);
    }
    else {
        scale->m_cfg_target_scale_x = NULL;
    }
     
    return 0;
}

int ui_sprite_2d_scale_set_target_scale_y(ui_sprite_2d_scale_t scale, const char * target_scale_y) {
    if (scale->m_cfg_target_scale_y) {
        mem_free(scale->m_module->m_alloc, scale->m_cfg_target_scale_y);
    }

    if (target_scale_y) {
        scale->m_cfg_target_scale_y = cpe_str_mem_dup_trim(scale->m_module->m_alloc, target_scale_y);
    }
    else {
        scale->m_cfg_target_scale_y = NULL;
    }
     
    return 0;
}

int ui_sprite_2d_scale_set_step(ui_sprite_2d_scale_t scale, const char * step) {
    if (scale->m_cfg_step) {
        mem_free(scale->m_module->m_alloc, scale->m_cfg_step);
    }

    if (step) {
        scale->m_cfg_step = cpe_str_mem_dup_trim(scale->m_module->m_alloc, step);
    }
    else {
        scale->m_cfg_step = NULL;
    }
     
    return 0;
}

int ui_sprite_2d_scale_set_duration(ui_sprite_2d_scale_t scale, const char * duration) {
    if (scale->m_cfg_duration) {
        mem_free(scale->m_module->m_alloc, scale->m_cfg_duration);
    }

    if (duration) {
        scale->m_cfg_duration = cpe_str_mem_dup_trim(scale->m_module->m_alloc, duration);
    }
    else {
        scale->m_cfg_duration = NULL;
    }

    return 0;
}

int ui_sprite_2d_scale_enter(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_2d_scale_t scale = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_2d_module_t module = scale->m_module;
	ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_2d_transform_t transform = ui_sprite_2d_transform_find(entity);

    if (transform == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): scale: no transform",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }

    if (scale->m_cfg_target_scale_x == NULL && scale->m_cfg_target_scale_y == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): scale: no target scale configured!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }

    if (scale->m_cfg_step == NULL && scale->m_cfg_duration == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): scale: no step or duration configured!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }

    if (scale->m_cfg_step && scale->m_cfg_duration) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): scale: all step and duration configured!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }

    scale->m_origin_scale = ui_sprite_2d_transform_scale_pair(transform);
    scale->m_target_scale = scale->m_origin_scale;
    
    if (scale->m_cfg_target_scale_x) {
        if (ui_sprite_fsm_action_check_calc_float(&scale->m_target_scale.x, scale->m_cfg_target_scale_x, fsm_action, NULL, module->m_em) != 0) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): scale: no target scale x from %s fail!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), scale->m_cfg_target_scale_x);
            return -1;
        }
    }

    if (scale->m_cfg_target_scale_y) {
        if (ui_sprite_fsm_action_check_calc_float(&scale->m_target_scale.y, scale->m_cfg_target_scale_y, fsm_action, NULL, module->m_em) != 0) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): scale: no target scale y from %s fail!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), scale->m_cfg_target_scale_y);
            return -1;
        }
    }

    if (scale->m_cfg_step) {
        float step;
        if (ui_sprite_fsm_action_check_calc_float(&step, scale->m_cfg_step, fsm_action, NULL, module->m_em) != 0) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): scale: calc step from %s fail!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), scale->m_cfg_step);
            return -1;
        }

        if (step <= 0.0f) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): scale: step %f error!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), step);
            return -1;
        }

        scale->m_work_duration.x = fabs(scale->m_target_scale.x - scale->m_origin_scale.x) / step;
        scale->m_work_duration.y = fabs(scale->m_target_scale.y - scale->m_origin_scale.y) / step;
    }

    if (scale->m_cfg_duration) {
        float duration;
        if (ui_sprite_fsm_action_check_calc_float(&duration, scale->m_cfg_duration, fsm_action, NULL, module->m_em) != 0) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): scale: calc duration from %s fail!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), scale->m_cfg_duration);
            return -1;
        }

        if (duration <= 0.0f) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): scale: duration %f error!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), duration);
            return -1;
        }

        scale->m_work_duration.x = scale->m_work_duration.y = duration;
    }

    scale->m_runing_time = 0.0f;

	ui_sprite_fsm_action_start_update(fsm_action);

    return 0;
}

void ui_sprite_2d_scale_update(ui_sprite_fsm_action_t fsm_action, void * ctx, float delta) {
    ui_sprite_2d_module_t module = ctx;
	ui_sprite_2d_scale_t scale = ui_sprite_fsm_action_data(fsm_action);
	ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
	ui_sprite_2d_transform_t transform;
    ui_vector_2 cur_scale;

    transform = ui_sprite_2d_transform_find(entity);
    if (transform == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): scale: no transform",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        ui_sprite_fsm_action_stop_update(fsm_action);
        return ;
    }

    cur_scale = ui_sprite_2d_transform_scale_pair(transform);
    
    scale->m_runing_time += delta;

    if (scale->m_work_duration.x > 0.0f) {
        if (scale->m_runing_time >= scale->m_work_duration.x) {
            cur_scale.x = scale->m_target_scale.x;
        }
        else {
            float percent = 0.0;
        
            percent = scale->m_runing_time / scale->m_work_duration.x;
            percent = ui_percent_decorator_decorate(&scale->m_cfg_decorator, percent);
            cur_scale.x = scale->m_origin_scale.x + (scale->m_target_scale.x - scale->m_origin_scale.x) * percent;
        }
    }

    if (scale->m_work_duration.y > 0.0f) {
        if (scale->m_runing_time >= scale->m_work_duration.y) {
            cur_scale.y = scale->m_target_scale.y;
        }
        else {
            float percent = 0.0;
        
            percent = scale->m_runing_time / scale->m_work_duration.y;
            percent = ui_percent_decorator_decorate(&scale->m_cfg_decorator, percent);
            cur_scale.y = scale->m_origin_scale.y + (scale->m_target_scale.y - scale->m_origin_scale.y) * percent;
        }
    }
    
    ui_sprite_2d_transform_set_scale_pair(transform, cur_scale);

    if (scale->m_runing_time >= scale->m_work_duration.x && scale->m_runing_time >= scale->m_work_duration.y) {
        ui_sprite_fsm_action_stop_update(fsm_action);
    }
}

void ui_sprite_2d_scale_exit(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_2d_module_t module = ctx;
    ui_sprite_2d_scale_t scale = ui_sprite_fsm_action_data(fsm_action);
	ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_2d_transform_t transform = ui_sprite_2d_transform_find(entity);

    if (transform == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): scale: no transform",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return;
    }

    ui_sprite_2d_transform_set_scale_pair(transform, scale->m_target_scale);
}

int ui_sprite_2d_scale_init(ui_sprite_fsm_action_t fsm_action, void * ctx) {
	ui_sprite_2d_scale_t scale = ui_sprite_fsm_action_data(fsm_action);

	bzero(scale, sizeof(*scale));
	scale->m_module = ctx;
    
	return 0;
}

int ui_sprite_2d_scale_copy(ui_sprite_fsm_action_t to, ui_sprite_fsm_action_t from, void * ctx) {
    ui_sprite_2d_module_t module = ctx;
    ui_sprite_2d_scale_t to_scale_to = ui_sprite_fsm_action_data(to);
    ui_sprite_2d_scale_t from_scale_to = ui_sprite_fsm_action_data(from);

    ui_sprite_2d_scale_init(to, ctx);

    memcpy(&to_scale_to->m_cfg_decorator, &from_scale_to->m_cfg_decorator, sizeof(to_scale_to->m_cfg_decorator));

    if (from_scale_to->m_cfg_target_scale_x) {
        to_scale_to->m_cfg_target_scale_x = cpe_str_mem_dup(module->m_alloc, from_scale_to->m_cfg_target_scale_x);
    }

    if (from_scale_to->m_cfg_target_scale_y) {
        to_scale_to->m_cfg_target_scale_y = cpe_str_mem_dup(module->m_alloc, from_scale_to->m_cfg_target_scale_y);
    }
    
    if (from_scale_to->m_cfg_duration) {
        to_scale_to->m_cfg_duration = cpe_str_mem_dup(module->m_alloc, from_scale_to->m_cfg_duration);
    }
    
    if (from_scale_to->m_cfg_step) {
        to_scale_to->m_cfg_step = cpe_str_mem_dup(module->m_alloc, from_scale_to->m_cfg_step);
    }
    
    return 0;
}

void ui_sprite_2d_scale_clear(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_2d_module_t module = ctx;
    ui_sprite_2d_scale_t scale = ui_sprite_fsm_action_data(fsm_action);

    if (scale->m_cfg_target_scale_x) {
        mem_free(module->m_alloc, scale->m_cfg_target_scale_x);
        scale->m_cfg_target_scale_x = NULL;
    }

    if (scale->m_cfg_target_scale_y) {
        mem_free(module->m_alloc, scale->m_cfg_target_scale_y);
        scale->m_cfg_target_scale_y = NULL;
    }
    
    if (scale->m_cfg_duration) {
        mem_free(module->m_alloc, scale->m_cfg_duration);
        scale->m_cfg_duration = NULL;
    }
    
    if (scale->m_cfg_step) {
        mem_free(module->m_alloc, scale->m_cfg_step);
        scale->m_cfg_step = NULL;
    }
}

ui_sprite_fsm_action_t ui_sprite_2d_scale_load(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg) {
	ui_sprite_2d_module_t module = ctx;
	ui_sprite_2d_scale_t scale;
    const char * str_value;

    scale = ui_sprite_2d_scale_create(fsm_state, name);
	if (scale == NULL) {
		CPE_ERROR(module->m_em, "%s: create 2d-scale action: create fail!", ui_sprite_2d_module_name(module));
		return NULL;
	}

    if ((str_value = cfg_get_string(cfg, "decorator", NULL))) {
        if (ui_sprite_2d_scale_set_decorator(scale, str_value) != 0) {
            CPE_ERROR(module->m_em, "%s: create 2d-scale action: set-decorator %s fail!", ui_sprite_2d_module_name(module), str_value);
            ui_sprite_2d_scale_free(scale);
            return NULL;
        }
    }

    if ((str_value = cfg_get_string(cfg, "target-scale", NULL))) {
        ui_sprite_2d_scale_set_target_scale_x(scale, str_value);
        ui_sprite_2d_scale_set_target_scale_y(scale, str_value);
    }
    else {
        uint8_t setted = 0;
        
        if ((str_value = cfg_get_string(cfg, "target-scale.x", NULL))) {
            ui_sprite_2d_scale_set_target_scale_x(scale, str_value);
            setted = 1;
        }

        if ((str_value = cfg_get_string(cfg, "target-scale.y", NULL))) {
            ui_sprite_2d_scale_set_target_scale_x(scale, str_value);
            setted = 1;
        }

        if (!setted) {
            CPE_ERROR(module->m_em, "%s: create 2d-scale action: target scale not configured!", ui_sprite_2d_module_name(module));
            ui_sprite_2d_scale_free(scale);
            return NULL;
        }
    }

    if ((str_value = cfg_get_string(cfg, "work-duration", NULL))) {
        ui_sprite_2d_scale_set_duration(scale, str_value);
    }

    if ((str_value = cfg_get_string(cfg, "work-step", NULL))) {
        ui_sprite_2d_scale_set_step(scale, str_value);
    }
    
	return ui_sprite_fsm_action_from_data(scale);
}

int ui_sprite_2d_scale_regist(ui_sprite_2d_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_create(
        module->m_fsm_module, UI_SPRITE_2D_SCALE_NAME, sizeof(struct ui_sprite_2d_scale));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: %s register: meta create fail",
            ui_sprite_2d_module_name(module), UI_SPRITE_2D_SCALE_NAME);
        return -1;
    }

    ui_sprite_fsm_action_meta_set_init_fun(meta, ui_sprite_2d_scale_init, module);
    ui_sprite_fsm_action_meta_set_copy_fun(meta, ui_sprite_2d_scale_copy, module);
    ui_sprite_fsm_action_meta_set_enter_fun(meta, ui_sprite_2d_scale_enter, module);
    ui_sprite_fsm_action_meta_set_exit_fun(meta, ui_sprite_2d_scale_exit, module);
    ui_sprite_fsm_action_meta_set_free_fun(meta, ui_sprite_2d_scale_clear, module);
	ui_sprite_fsm_action_meta_set_update_fun(meta, ui_sprite_2d_scale_update, module);

    if (module->m_loader) {
        if (ui_sprite_cfg_loader_add_action_loader(module->m_loader, UI_SPRITE_2D_SCALE_NAME, ui_sprite_2d_scale_load, module) != 0) {
            ui_sprite_fsm_action_meta_free(meta);
            return -1;
        }
    }
    
    return 0;
}

void ui_sprite_2d_scale_unregist(ui_sprite_2d_module_t module) {
	ui_sprite_fsm_action_meta_t meta;

	meta = ui_sprite_fsm_action_meta_find(module->m_fsm_module, UI_SPRITE_2D_SCALE_NAME);
	if (meta == NULL) {
		CPE_ERROR(
			module->m_em, "%s: %s unregister: meta not exist",
			ui_sprite_2d_module_name(module), UI_SPRITE_2D_SCALE_NAME);
		return;
	}

	ui_sprite_fsm_action_meta_free(meta);

    if (module->m_loader) {
        ui_sprite_cfg_loader_remove_action_loader(module->m_loader, UI_SPRITE_2D_SCALE_NAME);
    }
}

const char * UI_SPRITE_2D_SCALE_NAME = "2d-scale";
