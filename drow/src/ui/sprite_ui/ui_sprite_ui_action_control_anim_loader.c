#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "gd/app/app_context.h"
#include "plugin/ui/plugin_ui_module.h"
#include "plugin/ui/plugin_ui_animation.h"
#include "plugin/ui/plugin_ui_anim_control_move_in.h"
#include "plugin/ui/plugin_ui_anim_control_move_out.h"
#include "plugin/ui/plugin_ui_anim_control_alpha_in.h"
#include "plugin/ui/plugin_ui_anim_control_alpha_out.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui_sprite_ui_action_control_anim_i.h"
#include "ui_sprite_ui_env_i.h"

static
ui_sprite_fsm_action_t
ui_sprite_ui_action_control_anim_load_i(
    ui_sprite_ui_module_t module, ui_sprite_fsm_state_t fsm_state, const char * name, const char * anim, cfg_t cfg)
{
    ui_sprite_ui_action_control_anim_t control_anim = ui_sprite_ui_action_control_anim_create(fsm_state, name);
    const char * str_value;
    
    if (control_anim == NULL) {
        CPE_ERROR(module->m_em, "%s: create action_control_anim action: create fail!", ui_sprite_ui_module_name(module));
        return NULL;
    }

    if ((str_value = cfg_get_string(cfg, "control", NULL))) {
        if (ui_sprite_ui_action_control_anim_set_control(control_anim, str_value) != 0) {
            CPE_ERROR(module->m_em, "ui_sprite_ui_action_control_anim_load: set control %s fail!", str_value);
            ui_sprite_ui_action_control_anim_free(control_anim);
            return NULL;
        }
    }
    else {
        CPE_ERROR(module->m_em, "ui_sprite_ui_action_control_anim_load: control not configured!");
        ui_sprite_ui_action_control_anim_free(control_anim);
        return NULL;
    }

    if ((str_value = cfg_get_string(cfg, "init", NULL))) {
        if (ui_sprite_ui_action_control_anim_set_init(control_anim, str_value) != 0) {
            CPE_ERROR(module->m_em, "ui_sprite_ui_action_control_anim_load: set init %s fail!", str_value);
            ui_sprite_ui_action_control_anim_free(control_anim);
            return NULL;
        }
    }
    
    if (ui_sprite_ui_action_control_anim_set_anim(control_anim, anim) != 0) {
        CPE_ERROR(module->m_em, "ui_sprite_ui_action_control_anim_load: set anim %s fail!", anim);
        ui_sprite_ui_action_control_anim_free(control_anim);
        return NULL;
    }

    control_anim->m_cfg_delay_ms = cfg_get_float(cfg, "delay", 0.0f);
    if (control_anim->m_cfg_delay_ms == 0.0f) {
        control_anim->m_cfg_delay_ms = ((float)cfg_get_uint32(cfg, "delay-frame", 0)) / plugin_ui_module_cfg_fps(module->m_ui_module);
    }

    if (cfg_try_get_uint32(cfg, "loop-count", &control_anim->m_cfg_loop_count) == 0) {
        control_anim->m_cfg_loop_delay_ms = cfg_get_float(cfg, "loop-delay", 0.0f);
        if (control_anim->m_cfg_loop_delay_ms == 0.0f) {
            control_anim->m_cfg_loop_delay_ms = ((float)cfg_get_uint32(cfg, "loop-delay-frame", 0)) / plugin_ui_module_cfg_fps(module->m_ui_module);
        }
    }
    else {
        control_anim->m_cfg_loop_count = 1;
        control_anim->m_cfg_loop_delay_ms = 0.0f;
    }
    
    return ui_sprite_fsm_action_from_data(control_anim);
    
}

ui_sprite_fsm_action_t
ui_sprite_ui_action_control_anim_load(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg) {
    ui_sprite_ui_module_t module = ctx;
    const char * str_value;

    str_value = cfg_get_string(cfg, "anim", NULL);
    if (str_value == NULL) {
        CPE_ERROR(module->m_em, "ui_sprite_ui_action_control_anim_load: anim not configured!");
        return NULL;
    }

    return ui_sprite_ui_action_control_anim_load_i(module, fsm_state, name, str_value, cfg);
}

ui_sprite_fsm_action_t
ui_sprite_ui_action_control_move_inout_load(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg) {
    ui_sprite_ui_module_t module = ctx;
    const char * str_value;
    const char * way;
    float take_time;
    mem_buffer_t anim_buffer;
    char buf[32];
    
    str_value = cfg_get_string(cfg, "way", NULL);
    if (str_value == NULL) {
        CPE_ERROR(module->m_em, "ui_sprite_ui_action_control_move_inout_load: way not configured!");
        return NULL;
    }

    if (strcmp(str_value, "in") == 0) {
        way = PLUGIN_UI_ANIM_CONTROL_MOVE_IN;
    }
    else if (strcmp(str_value, "out") == 0) {
        way = PLUGIN_UI_ANIM_CONTROL_MOVE_OUT;
    }
    else {
        CPE_ERROR(module->m_em, "ui_sprite_ui_action_control_move_inout_load: way %s unknown!", str_value);
        return NULL;
    }

    /*构造anim字段 */
    /*    example: control-move-out: control-move-out.take-time=0.3, control-move-out.decorator=ease-back-out, control-move-out.policy=left*/
    anim_buffer = gd_app_tmp_buffer(module->m_app);
    mem_buffer_clear_data(anim_buffer);

    mem_buffer_strcat(anim_buffer, way);
    mem_buffer_strcat(anim_buffer, ": ");

    take_time = cfg_get_float(cfg, "take-time", 0.0f);
    if (take_time == 0.0f) {
        CPE_ERROR(module->m_em, "ui_sprite_ui_action_control_move_inout_load: take-time not configured!");
        return NULL;
    }
    snprintf(buf, sizeof(buf), "%f", take_time);
    mem_buffer_strcat(anim_buffer, way);
    mem_buffer_strcat(anim_buffer, ".take-time=");
    mem_buffer_strcat(anim_buffer, buf);
    
    if ((str_value = cfg_get_string(cfg, "decorator", NULL))) {
        mem_buffer_strcat(anim_buffer, ", ");
        mem_buffer_strcat(anim_buffer, way);
        mem_buffer_strcat(anim_buffer, ".decorator=");
        mem_buffer_strcat(anim_buffer, str_value);
    }

    if ((str_value = cfg_get_string(cfg, "policy", NULL))) {
        mem_buffer_strcat(anim_buffer, ", ");
        mem_buffer_strcat(anim_buffer, way);
        if (way == PLUGIN_UI_ANIM_CONTROL_MOVE_IN) {
            mem_buffer_strcat(anim_buffer, ".origin=");
        }
        else {
            mem_buffer_strcat(anim_buffer, ".target=");
        }
        mem_buffer_strcat(anim_buffer, str_value);
    }
    else {
        CPE_ERROR(module->m_em, "ui_sprite_ui_action_control_move_inout_load: policy not configured!");
        return NULL;
    }
    
    mem_buffer_append_char(anim_buffer, 0);
    str_value = mem_buffer_make_continuous(anim_buffer, 0);

    return ui_sprite_ui_action_control_anim_load_i(module, fsm_state, name, str_value, cfg);
}

ui_sprite_fsm_action_t
ui_sprite_ui_action_control_alpha_inout_load(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg) {
    ui_sprite_ui_module_t module = ctx;
    const char * str_value;
    float float_value;
    const char * way;
    mem_buffer_t anim_buffer;
    char buf[32];
    
    str_value = cfg_get_string(cfg, "way", NULL);
    if (str_value == NULL) {
        CPE_ERROR(module->m_em, "ui_sprite_ui_action_control_anim_load: way not configured!");
        return NULL;
    }

    if (strcmp(str_value, "in") == 0) {
        way = PLUGIN_UI_ANIM_CONTROL_ALPHA_IN;
    }
    else if (strcmp(str_value, "out") == 0) {
        way = PLUGIN_UI_ANIM_CONTROL_ALPHA_OUT;
    }
    else {
        CPE_ERROR(module->m_em, "ui_sprite_ui_action_control_alpha_inout_load: way %s unknown!", str_value);
        return NULL;
    }
    
    /*构造anim字段 */
    /*    example: control-alpha-out: control-alpha-out.take-time=0.3, control-alpha-out.decorator=ease-back-out, control-alpha-out.policy=left*/
    anim_buffer = gd_app_tmp_buffer(module->m_app);
    mem_buffer_clear_data(anim_buffer);

    mem_buffer_strcat(anim_buffer, way);
    mem_buffer_strcat(anim_buffer, ": ");

    float_value = cfg_get_float(cfg, "take-time", 0.0f);
    if (float_value == 0.0f) {
        CPE_ERROR(module->m_em, "ui_sprite_ui_action_control_alpha_inout_load: take-time not configured!");
        return NULL;
    }
    snprintf(buf, sizeof(buf), "%f", float_value);
    mem_buffer_strcat(anim_buffer, way);
    mem_buffer_strcat(anim_buffer, ".take-time=");
    mem_buffer_strcat(anim_buffer, buf);
    
    if ((str_value = cfg_get_string(cfg, "decorator", NULL))) {
        mem_buffer_strcat(anim_buffer, ", ");
        mem_buffer_strcat(anim_buffer, way);
        mem_buffer_strcat(anim_buffer, ".decorator=");
        mem_buffer_strcat(anim_buffer, str_value);
    }

    if (cfg_try_get_float(cfg, "origin", &float_value)) {
        snprintf(buf, sizeof(buf), "%f", float_value);
        mem_buffer_strcat(anim_buffer, ", ");
        mem_buffer_strcat(anim_buffer, way);
        mem_buffer_strcat(anim_buffer, ".origin=");
        mem_buffer_strcat(anim_buffer, buf);
    }

    if (cfg_try_get_float(cfg, "target", &float_value)) {
        snprintf(buf, sizeof(buf), "%f", float_value);
        mem_buffer_strcat(anim_buffer, ", ");
        mem_buffer_strcat(anim_buffer, way);
        mem_buffer_strcat(anim_buffer, ".target=");
        mem_buffer_strcat(anim_buffer, buf);
    }

    mem_buffer_append_char(anim_buffer, 0);
    str_value = mem_buffer_make_continuous(anim_buffer, 0);

    return ui_sprite_ui_action_control_anim_load_i(module, fsm_state, name, str_value, cfg);
}
