#include <assert.h>
#include "spine/Skeleton.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "gd/app/app_log.h"
#include "plugin/spine/plugin_spine_obj.h"
#include "ui/sprite/ui_sprite_world.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite_tri/ui_sprite_tri_action.h"
#include "ui/sprite_tri/ui_sprite_tri_action_remove_self.h"
#include "ui/sprite_tri/ui_sprite_tri_action_send_event.h"
#include "ui/sprite_spine/ui_sprite_spine_tri_apply_transition.h"
#include "ui/sprite_spine/ui_sprite_spine_tri_set_timescale.h"
#include "ui_sprite_spine_chipmunk_with_tri_i.h"

#ifdef __cplusplus
extern "C" {
#endif

int ui_sprite_spine_chipmunk_with_tri_build_action(
    ui_sprite_spine_chipmunk_module_t module, ui_sprite_entity_t entity, plugin_spine_obj_t spine_obj,
    ui_sprite_spine_chipmunk_with_tri_t with_tri, spSlot * slot,
    ui_sprite_tri_rule_t rule, ui_sprite_tri_action_trigger_t trigger, char * cfg)
{
    ui_sprite_tri_action_t action;
    char * args;
    char * str_value;
    char empty_buf[] = "";
    
    args = strchr(cfg, ':');
    if (args == NULL) {
        args = empty_buf;
    }
    else {
        *args = 0;
        args = cpe_str_trim_head(args + 1);
    }
    
    if (strcmp(cfg, "P_A") == 0) {
        ui_sprite_spine_tri_apply_transition_t apply_transition = ui_sprite_spine_tri_apply_transition_create(rule);
        if (apply_transition == NULL) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): spine with tri: create apply transition action fail!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
            return -1;
        }

        ui_sprite_spine_tri_apply_transition_set_obj(apply_transition, spine_obj);
        
        if ((str_value = cpe_str_read_and_remove_arg(args, "anim", ',', '='))) {
            char * sep = strchr(str_value, '.');
            if (sep == NULL) {
                CPE_ERROR(
                    module->m_em, "entity %d(%s): spine with tri: create apply transition action anim not configured!",
                    ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
                return -1;
            }

            *sep = 0;

            if (ui_sprite_spine_tri_apply_transition_set_part(apply_transition, str_value) != 0) {
                CPE_ERROR(
                    module->m_em, "entity %d(%s): spine with tri: create apply transition action set part %s fail!",
                    ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), str_value);
                return -1;
            }

            if (ui_sprite_spine_tri_apply_transition_set_transition(apply_transition, sep + 1) != 0) {
                CPE_ERROR(
                    module->m_em, "entity %d(%s): spine with tri: create apply transition action set transition %s fail!",
                    ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), sep + 1);
                return -1;
            }
        }
        else {
            CPE_ERROR(
                module->m_em, "entity %d(%s): spine with tri: create apply transition action anim not configured!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
            return -1;
        }

        action = ui_sprite_tri_action_from_data(apply_transition);
    }
    else if (strcmp(cfg, "P_T") == 0) {
        ui_sprite_spine_tri_set_timescale_t set_timescale = ui_sprite_spine_tri_set_timescale_create(rule);
        if (set_timescale == NULL) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): spine with tri: create apply timescale action fail!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
            return -1;
        }

        ui_sprite_spine_tri_set_timescale_set_obj(set_timescale, spine_obj);
        
        if ((str_value = cpe_str_read_and_remove_arg(args, "anim", ',', '='))) {
            char * sep = strchr(str_value, '.');
            if (sep == NULL) {
                CPE_ERROR(
                    module->m_em, "entity %d(%s): spine with tri: create apply timescale action anim not configured!",
                    ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
                return -1;
            }

            *sep = 0;

            if (ui_sprite_spine_tri_set_timescale_set_part(set_timescale, str_value) != 0) {
                CPE_ERROR(
                    module->m_em, "entity %d(%s): spine with tri: create apply timescale action set part %s fail!",
                    ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), str_value);
                return -1;
            }

            if (ui_sprite_spine_tri_set_timescale_set_timescale(set_timescale, sep + 1) != 0) {
                CPE_ERROR(
                    module->m_em, "entity %d(%s): spine with tri: create apply timescale action set timescale %s fail!",
                    ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), sep + 1);
                return -1;
            }
        }
        else {
            CPE_ERROR(
                module->m_em, "entity %d(%s): spine with tri: create apply transition action anim not configured!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
            return -1;
        }

        action = ui_sprite_tri_action_from_data(set_timescale);
    }
    else if (strcmp(cfg, "R_S") == 0) {
        ui_sprite_tri_action_remove_self_t remove_self = ui_sprite_tri_action_remove_self_create(rule);
        if (remove_self == NULL) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): spine with tri: create remove self action fail!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
            return -1;
        }

        action = ui_sprite_tri_action_from_data(remove_self);
    }
    else if (strcmp(cfg, "S_E") == 0) {
        ui_sprite_tri_action_send_event_t send_event = ui_sprite_tri_action_send_event_create(rule);
        if (send_event == NULL) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): spine with tri: create send event action fail!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
            return -1;
        }

        action = ui_sprite_tri_action_from_data(send_event);
    }
    else {
        CPE_ERROR(
            module->m_em, "entity %d(%s): spine with tri: unknown action type %s!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), cfg);
        return -1;
    }

    ui_sprite_tri_action_set_trigger(action, trigger);

    return 0;
}

#ifdef __cplusplus
}
#endif
