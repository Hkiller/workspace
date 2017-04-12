#include <assert.h>
#include "cpe/pal/pal_stdio.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/string_utils.h"
#include "cpe/dr/dr_data_value.h"
#include "render/utils/ui_string_table.h"
#include "plugin/ui/plugin_ui_env.h"
#include "plugin/ui/plugin_ui_animation.h"
#include "plugin/ui/plugin_ui_animation_meta.h"
#include "plugin/ui/plugin_ui_animation_control.h"
#include "plugin/ui/plugin_ui_control.h"
#include "plugin/ui/plugin_ui_control_frame.h"
#include "plugin_ui_anim_label_time_duration_i.h"

plugin_ui_anim_label_time_duration_t
plugin_ui_anim_label_time_duration_create(plugin_ui_env_t env) {
    plugin_ui_animation_t animation;

    animation = plugin_ui_animation_create_by_type_name(env, PLUGIN_UI_ANIM_LABEL_TIME_DURATION);
    if (animation == NULL) return NULL;
    
    return (plugin_ui_anim_label_time_duration_t)plugin_ui_animation_data(animation);
}

void plugin_ui_anim_label_time_duration_set_duration(plugin_ui_anim_label_time_duration_t time_duration, float duration) {
    time_duration->m_cfg_duration = duration;
}

void plugin_ui_anim_label_time_duration_set_formator(
    plugin_ui_anim_label_time_duration_t time_duration,
    plugin_ui_anim_label_time_duration_formator_category_t category, uint32_t msg_id)
{
    assert(category >= 0 && category <= CPE_ARRAY_SIZE(time_duration->m_cfg_formator));
    time_duration->m_cfg_formator[category] = msg_id;
}

int plugin_ui_anim_label_time_duration_set_done_res(
    plugin_ui_anim_label_time_duration_t time_duration, const char * done_res)
{
    char * new_res = NULL;
    if (done_res) {
        new_res = cpe_str_mem_dup(time_duration->m_module->m_alloc, done_res);
        if (new_res == NULL) {
            CPE_ERROR(time_duration->m_module->m_em, "plugin_ui_anim_label_time_duration_set_done_res: alloc fail!");
            return -1;
        }
    }

    if (time_duration->m_cfg_done_res) {
        mem_free(time_duration->m_module->m_alloc, time_duration->m_cfg_done_res);
    }

    time_duration->m_cfg_done_res = new_res;
    return 0;
}

int plugin_ui_anim_label_time_duration_init(plugin_ui_animation_t animation, void * ctx) {
    plugin_ui_module_t module = ctx;
    plugin_ui_anim_label_time_duration_t time_duration = plugin_ui_animation_data(animation);

    bzero(time_duration, sizeof(*time_duration));
    time_duration->m_module = module;
    return 0;
}
        
void plugin_ui_anim_label_time_duration_free(plugin_ui_animation_t animation, void * ctx) {
    plugin_ui_module_t module = ctx;
    plugin_ui_anim_label_time_duration_t time_duration = plugin_ui_animation_data(animation);

    if (time_duration->m_cfg_done_res) {
        mem_free(module->m_alloc, time_duration->m_cfg_done_res);
        time_duration->m_cfg_done_res = NULL;
    }
}

static void plugin_ui_anim_label_time_duration_update_controls(
    plugin_ui_module_t module, plugin_ui_control_t label, plugin_ui_anim_label_time_duration_t time_duration)
{
    float left_time_s;
    plugin_ui_anim_label_time_duration_formator_category_t category;
    uint8_t i;
    uint32_t msg_id;
    uint8_t left_sec = 0;
    uint8_t left_min = 0;
    uint8_t left_hor = 0;
    uint8_t left_day = 0;
    uint8_t left_mon = 0;
    uint8_t left_year = 0;
    
    /*已经结束 */
    if (time_duration->m_worked_duration >= time_duration->m_cfg_duration) {
        left_time_s = 0.0f;

        if (time_duration->m_cfg_done_res) {
            if (plugin_ui_control_set_attr_by_str(label, "text", "") != 0) {
                CPE_ERROR(
                    module->m_em, "plugin_ui_anim_label_time_duration: control %s set text to \"\" fail!",
                    plugin_ui_control_name(label));
            }

            if (plugin_ui_control_set_attr_by_str(label, "text-id", "") != 0) {
                CPE_ERROR(
                    module->m_em, "plugin_ui_anim_label_time_duration: control %s set text-id to \"\" fail!",
                    plugin_ui_control_name(label));
            }

            if (plugin_ui_control_frame_create_by_res(label, plugin_ui_control_frame_layer_back, plugin_ui_control_frame_usage_normal, time_duration->m_cfg_done_res) == NULL) {
                CPE_ERROR(
                    module->m_em, "plugin_ui_anim_label_time_duration: control %s set text to \"\" fail!",
                    plugin_ui_control_name(label));
            }

            return;
        }
    }
    else {
        left_time_s = time_duration->m_cfg_duration - time_duration->m_worked_duration;
    }

    if (time_duration->m_updated_duration == left_time_s) return;
    time_duration->m_updated_duration = left_time_s;
        
    if (left_time_s == 0.0f) {
        category = plugin_ui_anim_label_time_duration_formator_done;
    }
    else if (left_time_s < 60.0f) {
        left_sec = (uint8_t)left_time_s;
        category = plugin_ui_anim_label_time_duration_formator_sec;
    }
    else if (left_time_s < 60.0f * 60.0f) {
        left_min = (uint8_t)(left_time_s / 60);
        left_time_s -= left_min * 60.0f;
        
        left_sec = (uint8_t)left_time_s;
        
        category = plugin_ui_anim_label_time_duration_formator_min;
    }
    else if (left_time_s < 24 * 60.0f * 60.0f) {
        left_hor = (uint8_t)(left_time_s / (60 * 60));
        left_time_s -= left_hor * (60 * 60);
        
        left_min = (uint8_t)(left_time_s / 60);
        left_time_s -= left_min * 60.0f;
        
        left_sec = (uint8_t)left_time_s;

        category = plugin_ui_anim_label_time_duration_formator_hor;
    }
    else if (left_time_s < 30 * 24 * 60.0f * 60.0f) {
        left_day = (uint8_t)(left_time_s / (24 * 60 * 60));
        left_time_s -= left_day * (24 * 60 * 60);
        
        left_hor = (uint8_t)(left_time_s / (60 * 60));
        left_time_s -= left_hor * (60 * 60);
        
        left_min = (uint8_t)(left_time_s / 60);
        left_time_s -= left_min * 60.0f;
        
        left_sec = (uint8_t)left_time_s;

        category = plugin_ui_anim_label_time_duration_formator_day;
    }
    else if (left_time_s < 12 * 30 * 24 * 60.0f * 60.0f) {
        left_mon = (uint8_t)(left_time_s / (30 * 24 * 60 * 60));
        left_time_s -= left_mon * (30 * 24 * 60 * 60);
        
        left_day = (uint8_t)(left_time_s / (24 * 60 * 60));
        left_time_s -= left_day * (24 * 60 * 60);
        
        left_hor = (uint8_t)(left_time_s / (60 * 60));
        left_time_s -= left_hor * (60 * 60);
        
        left_min = (uint8_t)(left_time_s / 60);
        left_time_s -= left_min * 60.0f;
        
        left_sec = (uint8_t)left_time_s;

        category = plugin_ui_anim_label_time_duration_formator_min;
    }
    else {
        left_year = (uint8_t)(left_time_s / (12 * 30 * 24 * 60 * 60));
        left_time_s -= left_year * (12 * 30 * 24 * 60 * 60);
        
        left_mon = (uint8_t)(left_time_s / (30 * 24 * 60 * 60));
        left_time_s -= left_mon * (30 * 24 * 60 * 60);
        
        left_day = (uint8_t)(left_time_s / (24 * 60 * 60));
        left_time_s -= left_day * (24 * 60 * 60);
        
        left_hor = (uint8_t)(left_time_s / (60 * 60));
        left_time_s -= left_hor * (60 * 60);
        
        left_min = (uint8_t)(left_time_s / 60);
        left_time_s -= left_min * 60.0f;
        
        left_sec = (uint8_t)left_time_s;
        
        category = plugin_ui_anim_label_time_duration_formator_year;
    }

    for(i = category; i < CPE_ARRAY_SIZE(time_duration->m_cfg_formator); ++i) {
        msg_id = time_duration->m_cfg_formator[i];
        if (msg_id) break;
    }

    if (msg_id) {
        char args[128];
        const char * msg;
        
        snprintf(args, sizeof(args), "ss=%.2d,s=%d,mm=%.2d,m=%d,hh=%.2d,h=%d,d=%d,mon=%d,y=%d",
                 left_sec, left_sec, left_min, left_min, left_hor, left_hor, left_day, left_mon, left_year);

        msg = ui_string_table_message_format(
            plugin_ui_env_string_table(plugin_ui_control_env(label)), msg_id, args);

        if (plugin_ui_control_set_attr_by_str(label, "text", msg) != 0) {
            CPE_ERROR(
                module->m_em, "plugin_ui_anim_label_time_duration: control %s set text to \"%s\" fail!",
                plugin_ui_control_name(label), msg);
        }

        if (plugin_ui_control_set_attr_by_str(label, "text-id", msg) != 0) {
            CPE_ERROR(
                module->m_em, "plugin_ui_anim_label_time_duration: control %s set text-id to \"%s\" fail!",
                plugin_ui_control_name(label), msg);
        }
    }
    else {
        char msg[32];
        switch(category) {
        case plugin_ui_anim_label_time_duration_formator_done:
            msg[0] = 0;
            break;
        case plugin_ui_anim_label_time_duration_formator_sec:
            snprintf(msg, sizeof(msg), "%.2d", left_min);
            break;
        case plugin_ui_anim_label_time_duration_formator_min:
            snprintf(msg, sizeof(msg), "%.2d:%.2d", left_min, left_sec);
            break;
        case plugin_ui_anim_label_time_duration_formator_hor:
            snprintf(msg, sizeof(msg), "%.2d:%.2d:%.2d", left_hor, left_min, left_sec);
            break;
        case plugin_ui_anim_label_time_duration_formator_day:
            snprintf(msg, sizeof(msg), "%d day+", left_day);
            break;
        case plugin_ui_anim_label_time_duration_formator_mon:
            snprintf(msg, sizeof(msg), "%d month+", left_mon);
            break;
        case plugin_ui_anim_label_time_duration_formator_year:
            snprintf(msg, sizeof(msg), "%d year+", left_year);
            break;
        default:
            assert(0);
            msg[0] = 0;
        }

        if (plugin_ui_control_set_attr_by_str(label, "text", msg) != 0) {
            CPE_ERROR(
                module->m_em, "plugin_ui_anim_label_time_duration: control %s set text to \"%s\" fail!",
                plugin_ui_control_name(label), msg);
        }

        if (plugin_ui_control_set_attr_by_str(label, "text-id", msg) != 0) {
            CPE_ERROR(
                module->m_em, "plugin_ui_anim_label_time_duration: control %s set text-id to \"%s\" fail!",
                plugin_ui_control_name(label), msg);
        }

    }
}

int plugin_ui_anim_label_time_duration_enter(plugin_ui_animation_t animation, void * ctx) {
    plugin_ui_anim_label_time_duration_t time_duration = plugin_ui_animation_data(animation);
    plugin_ui_module_t module = (plugin_ui_module_t)ctx;
    plugin_ui_control_t label_control;

    time_duration->m_updated_duration = -1.0f;
    time_duration->m_worked_duration = 0.0f;
    
    label_control = plugin_ui_animation_find_first_tie_control(animation);
    if (label_control == NULL) {
        CPE_ERROR(module->m_em, "plugin_ui_anim_label_time_duration: label control not exist!");
        return -1;
    }
    
    plugin_ui_anim_label_time_duration_update_controls(module, label_control, time_duration);

    return 0;
}

void plugin_ui_anim_label_time_duration_exit(plugin_ui_animation_t animation, void * ctx) {
}

uint8_t plugin_ui_anim_label_time_duration_update(plugin_ui_animation_t animation, void * ctx, float delta_s) {
    plugin_ui_module_t module = (plugin_ui_module_t)ctx;
    plugin_ui_anim_label_time_duration_t time_duration = plugin_ui_animation_data(animation);
    plugin_ui_control_t label_control;

    time_duration->m_worked_duration += delta_s;
    
    label_control = plugin_ui_animation_find_first_tie_control(animation);
    if (label_control == NULL) return 0;

    plugin_ui_anim_label_time_duration_update_controls(module, label_control, time_duration);

    return time_duration->m_worked_duration >= time_duration->m_cfg_duration ? 0 : 1;
}

int plugin_ui_anim_label_time_duration_setup(
    plugin_ui_animation_t animation, void * ctx, char * arg_buf_will_change,
    plugin_ui_control_t control, plugin_ui_control_frame_t frame)
{
    plugin_ui_anim_label_time_duration_t time_duration = (plugin_ui_anim_label_time_duration_t)plugin_ui_animation_data(animation);
    plugin_ui_module_t module = time_duration->m_module;

    if (plugin_ui_animation_control_create(animation, control, 1) == NULL) {
        CPE_ERROR(module->m_em, "plugin_ui_anim_label_time_duration: add main control fail!");
        return -1;
    }
    
    return 0;
}

int plugin_ui_anim_label_time_duration_regist(plugin_ui_module_t module) {
    plugin_ui_animation_meta_t meta =
        plugin_ui_animation_meta_create(
            module,
            PLUGIN_UI_ANIM_LABEL_TIME_DURATION, module,
            /*animation*/
            sizeof(struct plugin_ui_anim_label_time_duration),
            plugin_ui_anim_label_time_duration_init,
            plugin_ui_anim_label_time_duration_free,
            plugin_ui_anim_label_time_duration_enter,
            plugin_ui_anim_label_time_duration_exit,
            plugin_ui_anim_label_time_duration_update,
            /*control*/
            0, NULL, NULL,
            plugin_ui_anim_label_time_duration_setup);
    
    return meta ? 0 : -1;
}

void plugin_ui_anim_label_time_duration_unregist(plugin_ui_module_t module) {
    plugin_ui_animation_meta_t meta =
        plugin_ui_animation_meta_find(
            module,
            PLUGIN_UI_ANIM_LABEL_TIME_DURATION);
    if (meta) {
        plugin_ui_animation_meta_free(meta);
    }
}

const char * PLUGIN_UI_ANIM_LABEL_TIME_DURATION = "label-time-duration";
