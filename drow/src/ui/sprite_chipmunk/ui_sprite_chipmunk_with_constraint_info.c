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
#include "ui/sprite_fsm/ui_sprite_fsm_action_calc.h"
#include "ui_sprite_chipmunk_with_constraint_i.h"
#include "ui_sprite_chipmunk_obj_i.h"
#include "ui_sprite_chipmunk_obj_body_i.h"
#include "ui_sprite_chipmunk_obj_constraint_i.h"

#ifdef __cplusplus
extern "C" {
#endif

ui_sprite_chipmunk_with_constraint_info_t
ui_sprite_chipmunk_with_constraint_info_create(
    ui_sprite_chipmunk_with_constraint_t with_constraint, const char * install_to, uint8_t constraint_type)
{
    size_t install_to_len = strlen(install_to) + 1;
    ui_sprite_chipmunk_with_constraint_info_t new_info =
        (ui_sprite_chipmunk_with_constraint_info_t)mem_calloc(
            with_constraint->m_module->m_alloc, sizeof(struct ui_sprite_chipmunk_with_constraint_info) + install_to_len);
    if (new_info == NULL) {
        CPE_ERROR(with_constraint->m_module->m_em, "chipmunk with monitor: create constraint info: alloc fail!")
        return NULL;
    }

    new_info->m_install_to = (char*)(new_info + 1);
    memcpy(new_info->m_install_to, install_to, install_to_len);
    new_info->m_constraint_type = constraint_type;
    TAILQ_INSERT_TAIL(&with_constraint->m_constraints, new_info, m_next);
    return new_info;
}

void ui_sprite_chipmunk_with_constraint_info_free(
    ui_sprite_chipmunk_with_constraint_t with_constraint, ui_sprite_chipmunk_with_constraint_info_t constraint_info)
{
    TAILQ_REMOVE(&with_constraint->m_constraints, constraint_info, m_next);

    switch(constraint_info->m_constraint_type) {
    case chipmunk_constraint_type_simple_motor:
        if (constraint_info->m_simple_motor.m_rate) {
            mem_free(with_constraint->m_module->m_alloc, constraint_info->m_simple_motor.m_rate);
            constraint_info->m_simple_motor.m_rate = NULL;
        }
        break;
    case chipmunk_constraint_type_rotary_limit_joint:
        if (constraint_info->m_rotary_limit.m_min) {
            mem_free(with_constraint->m_module->m_alloc, constraint_info->m_rotary_limit.m_min);
            constraint_info->m_rotary_limit.m_min = NULL;
        }
        if (constraint_info->m_rotary_limit.m_max) {
            mem_free(with_constraint->m_module->m_alloc, constraint_info->m_rotary_limit.m_max);
            constraint_info->m_rotary_limit.m_max = NULL;
        }
        break;
    }

    mem_free(with_constraint->m_module->m_alloc, constraint_info);
}
    
ui_sprite_chipmunk_with_constraint_info_t
ui_sprite_chipmunk_with_constraint_info_copy(
    ui_sprite_chipmunk_with_constraint_t with_constraint, ui_sprite_chipmunk_with_constraint_info_t from)
{
    ui_sprite_chipmunk_with_constraint_info_t new_info =
        ui_sprite_chipmunk_with_constraint_info_create(with_constraint, from->m_install_to, from->m_constraint_type);
    if (new_info == NULL) return NULL;

    switch(from->m_constraint_type) {
    case chipmunk_constraint_type_simple_motor:
        if (from->m_simple_motor.m_rate) {
            new_info->m_simple_motor.m_rate = cpe_str_mem_dup(with_constraint->m_module->m_alloc, from->m_simple_motor.m_rate);
            if (new_info->m_simple_motor.m_rate == NULL) {
                CPE_ERROR(with_constraint->m_module->m_em, "chipmunk with constraint: copy rate fail");
                ui_sprite_chipmunk_with_constraint_info_free(with_constraint, new_info);
                return NULL;
            }
        }
        break;
    case chipmunk_constraint_type_rotary_limit_joint:
        if (from->m_rotary_limit.m_min) {
            new_info->m_rotary_limit.m_min = cpe_str_mem_dup(with_constraint->m_module->m_alloc, from->m_rotary_limit.m_min);
            if (new_info->m_rotary_limit.m_min == NULL) {
                CPE_ERROR(with_constraint->m_module->m_em, "chipmunk with constraint: copy min fail");
                ui_sprite_chipmunk_with_constraint_info_free(with_constraint, new_info);
                return NULL;
            }
        }

        if (from->m_rotary_limit.m_max) {
            new_info->m_rotary_limit.m_max = cpe_str_mem_dup(with_constraint->m_module->m_alloc, from->m_rotary_limit.m_max);
            if (new_info->m_rotary_limit.m_max == NULL) {
                CPE_ERROR(with_constraint->m_module->m_em, "chipmunk with constraint: copy max fail");
                ui_sprite_chipmunk_with_constraint_info_free(with_constraint, new_info);
                return NULL;
            }
        }
        break;
    }
    
    return new_info;
}

#ifdef __cplusplus
}
#endif
    
