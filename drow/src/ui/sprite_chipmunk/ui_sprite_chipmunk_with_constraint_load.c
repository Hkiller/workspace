#include <assert.h>
#include "cpe/pal/pal_stdio.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/pal/pal_string.h"
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "render/model/ui_data_mgr.h"
#include "render/model/ui_data_src.h"
#include "plugin/chipmunk/plugin_chipmunk_module.h"
#include "plugin/chipmunk/plugin_chipmunk_env.h"
#include "ui/sprite/ui_sprite_world.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_meta.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui_sprite_chipmunk_with_constraint_i.h"
#include "ui_sprite_chipmunk_obj_i.h"
#include "ui_sprite_chipmunk_obj_body_i.h"
#include "ui_sprite_chipmunk_obj_constraint_i.h"

#ifdef __cplusplus
extern "C" {
#endif

ui_sprite_fsm_action_t
ui_sprite_chipmunk_with_constraint_load(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg) {
    ui_sprite_chipmunk_module_t module = (ui_sprite_chipmunk_module_t)ctx;
    ui_sprite_chipmunk_env_t env;
    ui_sprite_world_t world = ui_sprite_fsm_to_world(ui_sprite_fsm_state_fsm(fsm_state));
    ui_sprite_chipmunk_with_constraint_t with_constraint = (ui_sprite_chipmunk_with_constraint_t)ui_sprite_chipmunk_with_constraint_create(fsm_state, name);
    struct cfg_it constraint_it;
    cfg_t constraint_cfg;

    env = ui_sprite_chipmunk_env_find(world);
    if (env == NULL) {
        CPE_ERROR(module->m_em, "%s: create with_constraint action: env not exist!", ui_sprite_chipmunk_module_name(module));
        ui_sprite_chipmunk_with_constraint_free(with_constraint);
        return NULL;
    }
    
    if (with_constraint == NULL) {
        CPE_ERROR(module->m_em, "%s: create with_constraint action: create fail!", ui_sprite_chipmunk_module_name(module));
        ui_sprite_chipmunk_with_constraint_free(with_constraint);
        return NULL;
    }

    cfg_it_init(&constraint_it, cfg_find_cfg(cfg, "constraints"));
    while((constraint_cfg = cfg_it_next(&constraint_it))) {
        const char * install_to = cfg_get_string(constraint_cfg, "install-to", NULL);
        const char * type  = cfg_get_string(constraint_cfg, "type", NULL);
        ui_sprite_chipmunk_with_constraint_info_t constraint_info;

        if (type == NULL) {
            CPE_ERROR(module->m_em, "%s: create with_constraint action: constraints: type not configured!", ui_sprite_chipmunk_module_name(module));
            ui_sprite_chipmunk_with_constraint_free(with_constraint);
            return NULL;
        }

        if (install_to == NULL) {
            CPE_ERROR(module->m_em, "%s: create with_constraint action: constraints: install-to not configured!", ui_sprite_chipmunk_module_name(module));
            ui_sprite_chipmunk_with_constraint_free(with_constraint);
            return NULL;
        }

        if (strcmp(type, "simple-motor") == 0) {
            const char * rate = cfg_get_string(constraint_cfg, "rate", NULL);
            if (rate == NULL) {
                CPE_ERROR(module->m_em, "%s: create with_constraint action: constraints: rate not configured!", ui_sprite_chipmunk_module_name(module));
                ui_sprite_chipmunk_with_constraint_free(with_constraint);
                return NULL;
            }

            constraint_info =
                ui_sprite_chipmunk_with_constraint_info_create(with_constraint, install_to, chipmunk_constraint_type_simple_motor);
            if (constraint_info == NULL) {
                CPE_ERROR(
                    module->m_em, "%s: create with_constraint action: constraints: create constraint info fail!",
                    ui_sprite_chipmunk_module_name(module));
                ui_sprite_chipmunk_with_constraint_free(with_constraint);
                return NULL;
            }

            constraint_info->m_simple_motor.m_rate = cpe_str_mem_dup(module->m_alloc, rate);
            if (constraint_info->m_simple_motor.m_rate == NULL) {
                CPE_ERROR(
                    module->m_em, "%s: create with_constraint action: constraints: create rate fail!",
                    ui_sprite_chipmunk_module_name(module));
                ui_sprite_chipmunk_with_constraint_free(with_constraint);
                return NULL;
            }
        }
		else if (strcmp(type, "pivot-joint") == 0) {
			constraint_info =
				ui_sprite_chipmunk_with_constraint_info_create(with_constraint, install_to, chipmunk_constraint_type_pivot_joint);
			if (constraint_info == NULL) {
				CPE_ERROR(
					module->m_em, "%s: create with_constraint action: constraints: create constraint info (piovt) fail!",
					ui_sprite_chipmunk_module_name(module));
				ui_sprite_chipmunk_with_constraint_free(with_constraint);
				return NULL;
			}
		}
        else if (strcmp(type, "pin-joint") == 0) {
            constraint_info =
                ui_sprite_chipmunk_with_constraint_info_create(with_constraint, install_to, chipmunk_constraint_type_pin_joint);
            if (constraint_info == NULL) {
                CPE_ERROR(
                    module->m_em, "%s: create with_constraint action: constraints: create constraint info (pin) fail!",
                    ui_sprite_chipmunk_module_name(module));
                ui_sprite_chipmunk_with_constraint_free(with_constraint);
                return NULL;
            }
        }
        else if (strcmp(type, "rotary-limit-joint") == 0) {
            const char * min;
            const char * max;

            min = cfg_get_string(constraint_cfg, "min", NULL);
            if (min == NULL) {
                CPE_ERROR(module->m_em, "%s: create with_constraint action: constraints: min not configured!", ui_sprite_chipmunk_module_name(module));
                ui_sprite_chipmunk_with_constraint_free(with_constraint);
                return NULL;
            }

            max = cfg_get_string(constraint_cfg, "max", NULL);
            if (max == NULL) {
                CPE_ERROR(module->m_em, "%s: create with_constraint action: constraints: max not configured!", ui_sprite_chipmunk_module_name(module));
                ui_sprite_chipmunk_with_constraint_free(with_constraint);
                return NULL;
            }
            
            constraint_info =
                ui_sprite_chipmunk_with_constraint_info_create(with_constraint, install_to, chipmunk_constraint_type_rotary_limit_joint);
            if (constraint_info == NULL) {
                CPE_ERROR(
                    module->m_em, "%s: create with_constraint action: constraints: create constraint info (pin) fail!",
                    ui_sprite_chipmunk_module_name(module));
                ui_sprite_chipmunk_with_constraint_free(with_constraint);
                return NULL;
            }

            constraint_info->m_rotary_limit.m_min = cpe_str_mem_dup(module->m_alloc, min);
            constraint_info->m_rotary_limit.m_max = cpe_str_mem_dup(module->m_alloc, max);
            
            if (constraint_info->m_rotary_limit.m_min == NULL || constraint_info->m_rotary_limit.m_max == NULL) {
                CPE_ERROR(
                    module->m_em, "%s: create with_constraint action: constraints: create min or max fail!",
                    ui_sprite_chipmunk_module_name(module));
                ui_sprite_chipmunk_with_constraint_free(with_constraint);
                return NULL;
            }
        }
        else if (strcmp(type, "fix-rotation-joint") == 0) {
            constraint_info =
                ui_sprite_chipmunk_with_constraint_info_create(with_constraint, install_to, chipmunk_constraint_type_fix_rotation_joint);
            if (constraint_info == NULL) {
                CPE_ERROR(
                    module->m_em, "%s: create with_constraint action: constraints: create constraint info (pin) fail!",
                    ui_sprite_chipmunk_module_name(module));
                ui_sprite_chipmunk_with_constraint_free(with_constraint);
                return NULL;
            }
        }
        else {
            CPE_ERROR(
                module->m_em, "%s: create with_constraint action: constraints: create constraint: not support type %s!",
                ui_sprite_chipmunk_module_name(module), type);
            ui_sprite_chipmunk_with_constraint_free(with_constraint);
            return NULL;
        }
    }

    return ui_sprite_fsm_action_from_data(with_constraint);
}

#ifdef __cplusplus
}
#endif
    
