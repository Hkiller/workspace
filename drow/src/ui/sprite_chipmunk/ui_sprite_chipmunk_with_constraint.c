#include <assert.h>
#include "cpe/pal/pal_stdio.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/pal/pal_string.h"
#include "cpe/utils/string_utils.h"
#include "cpe/utils/math_ex.h"
#include "cpe/cfg/cfg_read.h"
#include "render/model/ui_data_mgr.h"
#include "render/model/ui_data_src.h"
#include "plugin/chipmunk/plugin_chipmunk_module.h"
#include "plugin/chipmunk/plugin_chipmunk_env.h"
#include "ui/sprite/ui_sprite_world.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_meta.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_calc.h"
#include "ui_sprite_chipmunk_with_constraint_i.h"
#include "ui_sprite_chipmunk_obj_i.h"
#include "ui_sprite_chipmunk_obj_body_i.h"
#include "ui_sprite_chipmunk_obj_constraint_i.h"

#ifdef __cplusplus
extern "C" {
#endif

static void ui_sprite_chipmunk_with_constraint_clear_constraints(
    ui_sprite_chipmunk_with_constraint_t with_constraint, ui_sprite_chipmunk_obj_t chipmunk_obj);
    
ui_sprite_chipmunk_with_constraint_t ui_sprite_chipmunk_with_constraint_create(ui_sprite_fsm_state_t fsm_state, const char * name) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_create(fsm_state, name, UI_SPRITE_CHIPMUNK_WITH_CONSTRAINT_NAME);
    return fsm_action ? (ui_sprite_chipmunk_with_constraint_t)ui_sprite_fsm_action_data(fsm_action) : NULL;
}

void ui_sprite_chipmunk_with_constraint_free(ui_sprite_chipmunk_with_constraint_t with_constraint) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_from_data(with_constraint);
    ui_sprite_fsm_action_free(fsm_action);
}

static int ui_sprite_chipmunk_with_constraint_enter(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_chipmunk_module_t module = (ui_sprite_chipmunk_module_t)ctx;
    ui_sprite_chipmunk_with_constraint_t with_constraint = (ui_sprite_chipmunk_with_constraint_t)ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_chipmunk_obj_t chipmunk_obj;
    ui_sprite_chipmunk_with_constraint_info_t constraint_info;
    cpSpace * space;
    
    chipmunk_obj = ui_sprite_chipmunk_obj_find(entity);
    if (chipmunk_obj == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): chipmunk with constraint: not chipmunk obj!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }

    space = (cpSpace * )plugin_chipmunk_env_space(chipmunk_obj->m_env->m_env);

    TAILQ_FOREACH(constraint_info, &with_constraint->m_constraints, m_next) {
        CHIPMUNK_CONSTRAINT_DATA constraint_data;

        bzero(&constraint_data, sizeof(constraint_data));

        switch(constraint_info->m_constraint_type) {
        case chipmunk_constraint_type_simple_motor:
            if (ui_sprite_fsm_action_try_calc_float(
                    &constraint_data.simple_motor.rate, constraint_info->m_simple_motor.m_rate, fsm_action, NULL, module->m_em)
                != 0)
            {
                CPE_ERROR(
                    module->m_em, "entity %d(%s): chipmunk with constraint: calc value %s fail!",
                    ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), constraint_info->m_simple_motor.m_rate);
                ui_sprite_chipmunk_with_constraint_clear_constraints(with_constraint, chipmunk_obj);
                return -1;
            }

            constraint_data.simple_motor.rate = cpe_math_angle_to_radians(constraint_data.simple_motor.rate);
            break;
        case chipmunk_constraint_type_rotary_limit_joint:
            if (ui_sprite_fsm_action_try_calc_float(
                    &constraint_data.rotary_limit_joint.min, constraint_info->m_rotary_limit.m_min, fsm_action, NULL, module->m_em)
                != 0)
            {
                CPE_ERROR(
                    module->m_em, "entity %d(%s): chipmunk with constraint: calc value %s fail!",
                    ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), constraint_info->m_rotary_limit.m_min);
                ui_sprite_chipmunk_with_constraint_clear_constraints(with_constraint, chipmunk_obj);
                return -1;
            }

            if (ui_sprite_fsm_action_try_calc_float(
                    &constraint_data.rotary_limit_joint.max, constraint_info->m_rotary_limit.m_max, fsm_action, NULL, module->m_em)
                != 0)
            {
                CPE_ERROR(
                    module->m_em, "entity %d(%s): chipmunk with constraint: calc value %s fail!",
                    ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), constraint_info->m_rotary_limit.m_max);
                ui_sprite_chipmunk_with_constraint_clear_constraints(with_constraint, chipmunk_obj);
                return -1;
            }

            break;
        default:
            break;
        };

        if (cpe_str_start_with(constraint_info->m_install_to, "constraint:")) {
            struct ui_sprite_chipmunk_obj_constraint_it constraint_it;
            ui_sprite_chipmunk_obj_constraint_t constraint;
            const char * constraint_prefix = constraint_info->m_install_to + strlen("constraint:");
  
            ui_sprite_chipmunk_obj_constraints(&constraint_it, chipmunk_obj);
            while((constraint = ui_sprite_chipmunk_obj_constraint_it_next(&constraint_it))) {
                ui_sprite_chipmunk_obj_constraint_t new_constraint;
                CHIPMUNK_CONSTRAINT_DATA const * from_constraint_info;
                char constraint_name[64];

                if (!cpe_str_start_with(constraint->m_name, constraint_prefix)) continue;

                from_constraint_info = ui_sprite_chipmunk_obj_constraint_data(constraint);
                
                switch(constraint_info->m_constraint_type) {
                case chipmunk_constraint_type_pin_joint:
                    if (constraint->m_constraint_type == chipmunk_constraint_type_pivot_joint) {
                        constraint_data.pin_joint.anchor_a = from_constraint_info->pivot_joint.anchor_a;
                        constraint_data.pin_joint.anchor_b = from_constraint_info->pivot_joint.anchor_b;
                    }
                    else {
                        CPE_ERROR(
                            module->m_em, "entity %d(%s): chipmunk with constraint: not support create pin from constraint %s type %d!",
                            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity),
                            constraint->m_name, constraint->m_constraint_type);
                        ui_sprite_chipmunk_with_constraint_clear_constraints(with_constraint, chipmunk_obj);
                        return -1;
                    }
                    break;
                case chipmunk_constraint_type_fix_rotation_joint:
                    break;
                default:
                    CPE_ERROR(
                        module->m_em, "entity %d(%s): chipmunk with constraint: not support create constraint type %d from constraint %s type %d!",
                        ui_sprite_entity_id(entity), ui_sprite_entity_name(entity),
                        constraint_info->m_constraint_type, constraint->m_name, constraint->m_constraint_type);
                    ui_sprite_chipmunk_with_constraint_clear_constraints(with_constraint, chipmunk_obj);
                    return -1;
                }
                
                snprintf(constraint_name, sizeof(constraint_name), "*%s", constraint->m_name);

                new_constraint = ui_sprite_chipmunk_obj_constraint_create(
                    constraint->m_body_a, constraint->m_body_b,
                    constraint_name, constraint_info->m_constraint_type, &constraint_data, 1);
                if (new_constraint == NULL) {
                    CPE_ERROR(
                        module->m_em, "entity %d(%s): chipmunk with constraint: create constraint %s fail!",
                        ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), constraint_name);
                    ui_sprite_chipmunk_with_constraint_clear_constraints(with_constraint, chipmunk_obj);
                    return -1;
                }

                new_constraint->m_creator = with_constraint;

                if (ui_sprite_chipmunk_obj_constraint_add_to_space(new_constraint, space) != 0) {
                    CPE_ERROR(
                        module->m_em, "entity %d(%s): chipmunk with constraint: add constraint %s to space fail!",
                        ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), constraint_name);
                    ui_sprite_chipmunk_with_constraint_clear_constraints(with_constraint, chipmunk_obj);
                    return -1;
                }
            }
        }
        else if (cpe_str_start_with(constraint_info->m_install_to, "body:")) {
            struct ui_sprite_chipmunk_obj_body_it body_it;
            ui_sprite_chipmunk_obj_body_t body;
            const char * body_prefix = constraint_info->m_install_to + strlen("body:");

            ui_sprite_chipmunk_obj_bodies(&body_it, chipmunk_obj);
            while((body = ui_sprite_chipmunk_obj_body_it_next(&body_it))) {
                ui_sprite_chipmunk_obj_constraint_t new_constraint;
                char constraint_name[64];

                if (!cpe_str_start_with(body->m_name, body_prefix)) continue;

                snprintf(constraint_name, sizeof(constraint_name), "*%s", body->m_name);

                new_constraint = ui_sprite_chipmunk_obj_constraint_create(
                    body, NULL,
                    constraint_name, constraint_info->m_constraint_type, &constraint_data, 1);
                if (new_constraint == NULL) {
                    CPE_ERROR(
                        module->m_em, "entity %d(%s): chipmunk with constraint: create constraint %s fail!",
                        ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), constraint_name);
                    ui_sprite_chipmunk_with_constraint_clear_constraints(with_constraint, chipmunk_obj);
                    return -1;
                }

                new_constraint->m_creator = with_constraint;

                if (ui_sprite_chipmunk_obj_constraint_add_to_space(new_constraint, space) != 0) {
                    CPE_ERROR(
                        module->m_em, "entity %d(%s): chipmunk with constraint: add constraint %s to space fail!",
                        ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), constraint_name);
                    ui_sprite_chipmunk_with_constraint_clear_constraints(with_constraint, chipmunk_obj);
                    return -1;
                }
            }
        }
    }
    
    return 0;
}

static void ui_sprite_chipmunk_with_constraint_clear_constraints(ui_sprite_chipmunk_with_constraint_t with_constraint, ui_sprite_chipmunk_obj_t chipmunk_obj) {
    struct ui_sprite_chipmunk_obj_constraint_it constraint_it;
    ui_sprite_chipmunk_obj_constraint_t constraint;
    ui_sprite_chipmunk_obj_constraint_t next_constraint;
    
    ui_sprite_chipmunk_obj_constraints(&constraint_it, chipmunk_obj);
    for(constraint = ui_sprite_chipmunk_obj_constraint_it_next(&constraint_it);
        constraint;
        constraint = next_constraint)
    {
        next_constraint = ui_sprite_chipmunk_obj_constraint_it_next(&constraint_it);

        if (constraint->m_creator == with_constraint) {
            if (constraint->m_is_in_space) {
                ui_sprite_chipmunk_obj_constraint_remove_from_space(constraint);
            }
            ui_sprite_chipmunk_obj_constraint_free(constraint);
        }
    }
}
    
static void ui_sprite_chipmunk_with_constraint_exit(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_chipmunk_module_t module = (ui_sprite_chipmunk_module_t)ctx;
    ui_sprite_chipmunk_with_constraint_t with_constraint = (ui_sprite_chipmunk_with_constraint_t)ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_chipmunk_obj_t chipmunk_obj;
    
    chipmunk_obj = ui_sprite_chipmunk_obj_find(entity);
    if (chipmunk_obj == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): chipmunk with velocity_fun: not chipmunk obj!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return;
    }

    ui_sprite_chipmunk_with_constraint_clear_constraints(with_constraint, chipmunk_obj);
}

static int ui_sprite_chipmunk_with_constraint_init(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_chipmunk_with_constraint_t with_constraint = (ui_sprite_chipmunk_with_constraint_t)ui_sprite_fsm_action_data(fsm_action);
    with_constraint->m_module = (ui_sprite_chipmunk_module_t)ctx;
    TAILQ_INIT(&with_constraint->m_constraints);
    return 0;
}

static void ui_sprite_chipmunk_with_constraint_clear(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    //ui_sprite_chipmunk_module_t module = (ui_sprite_chipmunk_module_t)ctx;
    ui_sprite_chipmunk_with_constraint_t with_constraint = (ui_sprite_chipmunk_with_constraint_t)ui_sprite_fsm_action_data(fsm_action);

    while(!TAILQ_EMPTY(&with_constraint->m_constraints)) {
        ui_sprite_chipmunk_with_constraint_info_t info = TAILQ_FIRST(&with_constraint->m_constraints);
        ui_sprite_chipmunk_with_constraint_info_free(with_constraint, info);
    }
}

static int ui_sprite_chipmunk_with_constraint_copy(ui_sprite_fsm_action_t to, ui_sprite_fsm_action_t from, void * ctx) {
    ui_sprite_chipmunk_with_constraint_t to_with_constraint = (ui_sprite_chipmunk_with_constraint_t)ui_sprite_fsm_action_data(to);
    ui_sprite_chipmunk_with_constraint_t from_with_constraint = (ui_sprite_chipmunk_with_constraint_t)ui_sprite_fsm_action_data(from);
    ui_sprite_chipmunk_with_constraint_info_t from_info;
    
    if (ui_sprite_chipmunk_with_constraint_init(to, ctx)) return -1;

    TAILQ_FOREACH(from_info, &from_with_constraint->m_constraints, m_next) {
        if (ui_sprite_chipmunk_with_constraint_info_copy(to_with_constraint, from_info) == NULL) {
            ui_sprite_chipmunk_with_constraint_clear(to, ctx);
            return -1;
        }
    }

    return 0;
}
    
int ui_sprite_chipmunk_with_constraint_regist(ui_sprite_chipmunk_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_create(
        module->m_fsm_module, UI_SPRITE_CHIPMUNK_WITH_CONSTRAINT_NAME, sizeof(struct ui_sprite_chipmunk_with_constraint));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: chipmunk with velocity_fun register: meta create fail",
            ui_sprite_chipmunk_module_name  (module));
        return -1;
    }

    ui_sprite_fsm_action_meta_set_enter_fun(meta, ui_sprite_chipmunk_with_constraint_enter, module);
    ui_sprite_fsm_action_meta_set_exit_fun(meta, ui_sprite_chipmunk_with_constraint_exit, module);
    ui_sprite_fsm_action_meta_set_init_fun(meta, ui_sprite_chipmunk_with_constraint_init, module);
    ui_sprite_fsm_action_meta_set_copy_fun(meta, ui_sprite_chipmunk_with_constraint_copy, module);
    ui_sprite_fsm_action_meta_set_free_fun(meta, ui_sprite_chipmunk_with_constraint_clear, module);

    if (ui_sprite_cfg_loader_add_action_loader(
            module->m_loader, UI_SPRITE_CHIPMUNK_WITH_CONSTRAINT_NAME, ui_sprite_chipmunk_with_constraint_load, module)
        != 0)
    {
        ui_sprite_fsm_action_meta_free(meta);
        return -1;
    }

    return 0;
}

void ui_sprite_chipmunk_with_constraint_unregist(ui_sprite_chipmunk_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_find(module->m_fsm_module, UI_SPRITE_CHIPMUNK_WITH_CONSTRAINT_NAME);
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: send event unregister: meta not exist",
            ui_sprite_chipmunk_module_name(module));
        return;
    }

    ui_sprite_fsm_action_meta_free(meta);

    ui_sprite_cfg_loader_remove_action_loader(module->m_loader, UI_SPRITE_CHIPMUNK_WITH_CONSTRAINT_NAME);
}

const char * UI_SPRITE_CHIPMUNK_WITH_CONSTRAINT_NAME = "chipmunk-with-constraint";

#ifdef __cplusplus
}
#endif
    
