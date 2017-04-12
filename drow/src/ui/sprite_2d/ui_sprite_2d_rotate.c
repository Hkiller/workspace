#include "cpe/pal/pal_strings.h"
#include "cpe/utils/math_ex.h"
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_meta.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_calc.h"
#include "ui/sprite_cfg/ui_sprite_cfg_loader.h"
#include "ui/sprite_2d/ui_sprite_2d_transform.h"
#include "ui_sprite_2d_rotate_i.h"
#include "ui_sprite_2d_module_i.h"

ui_sprite_2d_rotate_t
ui_sprite_2d_rotate_create(ui_sprite_fsm_state_t fsm_state, const char * name) {
    ui_sprite_fsm_action_t fsm_action;
    fsm_action = ui_sprite_fsm_action_create(fsm_state, name, UI_SPRITE_2D_ROTATE_NAME);
    return fsm_action ? ui_sprite_fsm_action_data(fsm_action) : NULL;
}

void ui_sprite_2d_rotate_free(ui_sprite_2d_rotate_t show_anim) {
    ui_sprite_fsm_action_free(ui_sprite_fsm_action_from_data(show_anim));
}

int ui_sprite_2d_rotate_set_op(ui_sprite_2d_rotate_t rotate, const char * op) {
    assert(op);

    if (rotate->m_cfg_op) {
        mem_free(rotate->m_module->m_alloc, rotate->m_cfg_op);
        rotate->m_cfg_op = NULL;
    }

    rotate->m_cfg_op = cpe_str_mem_dup(rotate->m_module->m_alloc, op);
    
    return 0;
}

int ui_sprite_2d_rotate_set_speed(ui_sprite_2d_rotate_t rotate, const char * speed) {
    assert(speed);

    if (rotate->m_cfg_speed) {
        mem_free(rotate->m_module->m_alloc, rotate->m_cfg_speed);
        rotate->m_cfg_speed = NULL;
    }

    rotate->m_cfg_speed = cpe_str_mem_dup(rotate->m_module->m_alloc, speed);
    
    return 0;
}

int ui_sprite_2d_rotate_set_decorator(ui_sprite_2d_rotate_t rotate, const char * decorator) {
    return ui_percent_decorator_setup(&rotate->m_cfg_decorator, decorator, rotate->m_module->m_em);
}

int ui_sprite_2d_rotate_enter(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_2d_module_t module = ctx;
    ui_sprite_2d_rotate_t rotate = ui_sprite_fsm_action_data(fsm_action);
	ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
	ui_sprite_2d_transform_t transform;
    struct mem_buffer buffer;
    const char * op;
    float speed;

    transform = ui_sprite_2d_transform_find(entity);
    if (transform == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): rotate: no transform!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }

    mem_buffer_init(&buffer, module->m_alloc);

    if (rotate->m_cfg_op == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): rotate: op not configured!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        goto ENTER_FAIL;
    }

    op = ui_sprite_fsm_action_check_calc_str(&buffer, rotate->m_cfg_op, fsm_action, NULL, module->m_em);
    if (op == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): rotate: calc op %s fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), rotate->m_cfg_op);
        goto ENTER_FAIL;
    }

    rotate->m_origin_angle = ui_sprite_2d_transform_angle(transform);
    if (op[0] == '+') {
        rotate->m_target_angle += rotate->m_origin_angle + atof(op + 1);
    }
    else if (op[0] == '-') {
        rotate->m_target_angle -= rotate->m_origin_angle + atof(op + 1);
    }
    else {
        rotate->m_target_angle = atof(op);
    }
    
    if (rotate->m_cfg_speed) {
        if (ui_sprite_fsm_action_check_calc_float(&speed, rotate->m_cfg_speed, fsm_action, NULL, module->m_em) != 0) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): rotate: calc speed %s fail!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), rotate->m_cfg_speed);
            goto ENTER_FAIL;
        }
    }
    else {
        speed = 0.0f;
    }

    if (speed <= 0.0f) {
        ui_sprite_2d_transform_set_angle(transform, rotate->m_target_angle);
    }
    else {
        rotate->m_runing_time = 0.0f;
        rotate->m_duration = fabs(rotate->m_target_angle - rotate->m_origin_angle) / speed;
        ui_sprite_fsm_action_start_update(fsm_action);
    }

    mem_buffer_clear(&buffer);
            
    return 0;

ENTER_FAIL:
    mem_buffer_clear(&buffer);

    return -1;
}

void ui_sprite_2d_rotate_exit(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_2d_rotate_t rotate = ui_sprite_fsm_action_data(fsm_action);    
	ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
	ui_sprite_2d_transform_t transform;

    transform = ui_sprite_2d_transform_find(entity);
    if (transform) {
        ui_sprite_2d_transform_set_angle(transform, rotate->m_target_angle);
    }
}

int ui_sprite_2d_rotate_init(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_2d_rotate_t rotate_to = ui_sprite_fsm_action_data(fsm_action);

    bzero(rotate_to, sizeof(*rotate_to));

    rotate_to->m_module = ctx;

    return 0;
}

int ui_sprite_2d_rotate_copy(ui_sprite_fsm_action_t to, ui_sprite_fsm_action_t from, void * ctx) {
	ui_sprite_2d_module_t module = ctx;
    ui_sprite_2d_rotate_t to_rotate_to = ui_sprite_fsm_action_data(to);
    ui_sprite_2d_rotate_t from_rotate_to = ui_sprite_fsm_action_data(from);

    ui_sprite_2d_rotate_init(to, ctx);
    memcpy(&to_rotate_to->m_cfg_decorator, &from_rotate_to->m_cfg_decorator, sizeof(to_rotate_to->m_cfg_decorator));

    if(from_rotate_to->m_cfg_op) {
        to_rotate_to->m_cfg_op = cpe_str_mem_dup(module->m_alloc, from_rotate_to->m_cfg_op);
    }

    if(from_rotate_to->m_cfg_speed) {
        to_rotate_to->m_cfg_speed = cpe_str_mem_dup(module->m_alloc, from_rotate_to->m_cfg_speed);
    }
    
    return 0;
}

void ui_sprite_2d_rotate_clear(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_2d_module_t module = ctx;
    ui_sprite_2d_rotate_t rotate = ui_sprite_fsm_action_data(fsm_action);
    
    if(rotate->m_cfg_op) {
        mem_free(module->m_alloc, rotate->m_cfg_op);
        rotate->m_cfg_op = NULL;
    }

    if(rotate->m_cfg_speed) {
        mem_free(module->m_alloc, rotate->m_cfg_speed);
        rotate->m_cfg_speed = NULL;
    }
}

void ui_sprite_2d_rotate_update(ui_sprite_fsm_action_t fsm_action, void * ctx, float delta) {
    ui_sprite_2d_module_t module = ctx;
    ui_sprite_2d_rotate_t rotate = ui_sprite_fsm_action_data(fsm_action);
	ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
	ui_sprite_2d_transform_t transform;
    float percent;
    
    rotate->m_runing_time += delta;
    
    transform = ui_sprite_2d_transform_find(entity);
    if (transform == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): rotate: no transform!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        ui_sprite_fsm_action_stop_update(fsm_action);
        return;
    }

    if (rotate->m_runing_time >= rotate->m_duration) {
        percent = 1.0f;
    }
    else {
        percent = rotate->m_runing_time / rotate->m_duration;
    }

    percent = ui_percent_decorator_decorate(&rotate->m_cfg_decorator, percent);
    ui_sprite_2d_transform_set_angle(transform, rotate->m_origin_angle + (rotate->m_target_angle - rotate->m_origin_angle) * percent);
    
    if (rotate->m_runing_time >= rotate->m_duration) {
        ui_sprite_fsm_action_stop_update(fsm_action);
    }
}

static ui_sprite_fsm_action_t ui_sprite_2d_rotate_load(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg) {
	ui_sprite_2d_module_t module = ctx;
	ui_sprite_2d_rotate_t p2d_rotate = ui_sprite_2d_rotate_create(fsm_state, name);
    const char * decorator;

	if (p2d_rotate == NULL) {
		CPE_ERROR(module->m_em, "%s: create anim_2d_rotate action: create fail!", ui_sprite_2d_module_name(module));
		return NULL;
	}

    if ((decorator = cfg_get_string(cfg, "decorator", NULL))) {
        if (ui_sprite_2d_rotate_set_decorator(p2d_rotate, decorator) != 0) {
            CPE_ERROR(module->m_em, "%s: create 2d rotate action: create fail!", ui_sprite_2d_module_name(module));
            ui_sprite_2d_rotate_free(p2d_rotate);
            return NULL;
        }
    }

	return ui_sprite_fsm_action_from_data(p2d_rotate);
}

int ui_sprite_2d_rotate_regist(ui_sprite_2d_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_create(
        module->m_fsm_module, UI_SPRITE_2D_ROTATE_NAME, sizeof(struct ui_sprite_2d_rotate));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: %s register: meta create fail",
            ui_sprite_2d_module_name(module), UI_SPRITE_2D_ROTATE_NAME);
        return -1;
    }

    ui_sprite_fsm_action_meta_set_init_fun(meta, ui_sprite_2d_rotate_init, module);
    ui_sprite_fsm_action_meta_set_copy_fun(meta, ui_sprite_2d_rotate_copy, module);
    ui_sprite_fsm_action_meta_set_enter_fun(meta, ui_sprite_2d_rotate_enter, module);
    ui_sprite_fsm_action_meta_set_exit_fun(meta, ui_sprite_2d_rotate_exit, module);
    ui_sprite_fsm_action_meta_set_free_fun(meta, ui_sprite_2d_rotate_clear, module);
    ui_sprite_fsm_action_meta_set_update_fun(meta, ui_sprite_2d_rotate_update, module);

    if (module->m_loader) {
        if (ui_sprite_cfg_loader_add_action_loader(module->m_loader, UI_SPRITE_2D_ROTATE_NAME, ui_sprite_2d_rotate_load, module) != 0) {
            ui_sprite_fsm_action_meta_free(meta);
            return -1;
        }
    }
    
    return 0;
}

void ui_sprite_2d_rotate_unregist(ui_sprite_2d_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_find(module->m_fsm_module, UI_SPRITE_2D_ROTATE_NAME);
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: %s unregister: meta not exist",
            ui_sprite_2d_module_name(module), UI_SPRITE_2D_ROTATE_NAME);
        return;
    }

    ui_sprite_fsm_action_meta_free(meta);

    if (module->m_loader) {
        ui_sprite_cfg_loader_remove_action_loader(module->m_loader, UI_SPRITE_2D_ROTATE_NAME);
    }
}

const char * UI_SPRITE_2D_ROTATE_NAME = "2d-rotate";
