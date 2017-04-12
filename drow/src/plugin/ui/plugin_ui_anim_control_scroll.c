#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/math_ex.h"
#include "cpe/utils/string_utils.h"
#include "render/model/ui_data_layout.h"
#include "plugin/ui/plugin_ui_animation.h"
#include "plugin/ui/plugin_ui_animation_meta.h"
#include "plugin/ui/plugin_ui_animation_control.h"
#include "plugin_ui_anim_control_scroll_i.h"
#include "plugin_ui_env_i.h"
#include "plugin_ui_control_i.h"
#include "plugin_ui_page_i.h"
#include "plugin_ui_utils_i.h"

static int plugin_ui_anim_control_scroll_setup(
    plugin_ui_animation_t animation, void * ctx, char * arg_buf_will_change,
    plugin_ui_control_t control, plugin_ui_control_frame_t frame);

plugin_ui_anim_control_scroll_t
plugin_ui_anim_control_scroll_create(plugin_ui_control_t control) {
    plugin_ui_env_t env = control->m_page->m_env;
    plugin_ui_animation_t animation;

    animation = plugin_ui_animation_create_by_type_name(env, PLUGIN_UI_ANIM_CONTROL_SCROLL);
    if (animation == NULL) return NULL;

    return plugin_ui_animation_data(animation);
}

plugin_ui_anim_control_scroll_t
plugin_ui_anim_control_scroll_create_with_setup(plugin_ui_control_t control, char * arg_buf_will_change) {
    plugin_ui_env_t env = control->m_page->m_env;
    plugin_ui_animation_t animation;

    animation = plugin_ui_animation_create_by_type_name(env, PLUGIN_UI_ANIM_CONTROL_SCROLL);
    if (animation == NULL) return NULL;

    if (plugin_ui_anim_control_scroll_setup(animation, env->m_module, arg_buf_will_change, control, NULL)) {
        plugin_ui_animation_free(animation);
        return NULL;
    }

    return plugin_ui_animation_data(animation);
}

int plugin_ui_anim_control_scroll_set_decorator(plugin_ui_anim_control_scroll_t move, const char * decorator) {
    if (ui_percent_decorator_setup(&move->m_cfg_percent_decorator, decorator, move->m_module->m_em) != 0) {
        CPE_ERROR(
            move->m_module->m_em, "%s: control-move-in: set decorator %s fail!",
            plugin_ui_module_name(move->m_module), decorator);
        return -1;
    }

    return 0;
}

uint8_t plugin_ui_anim_control_scroll_guard_done(plugin_ui_anim_control_scroll_t scroll) {
    return scroll->m_cfg_guard_done;
}

void plugin_ui_anim_control_scroll_set_guard_done(plugin_ui_anim_control_scroll_t scroll, uint8_t guard_done) {
    scroll->m_cfg_guard_done = guard_done;
}

void plugin_ui_anim_control_scroll_set_take_time(plugin_ui_anim_control_scroll_t move, float take_time) {
    move->m_cfg_take_time = take_time;
}

void plugin_ui_anim_control_scroll_set_target_x(plugin_ui_anim_control_scroll_t control_scroll, float x) {
    control_scroll->m_cfg_target_pos.x = x;
    control_scroll->m_cfg_process_x = 1;
}

uint8_t plugin_ui_anim_control_scroll_process_x(plugin_ui_anim_control_scroll_t control_scroll) {
    return control_scroll->m_cfg_process_x;
}

void plugin_ui_anim_control_scroll_set_target_y(plugin_ui_anim_control_scroll_t control_scroll, float y) {
    control_scroll->m_cfg_target_pos.y = y;
    control_scroll->m_cfg_process_y = 1;
}

uint8_t plugin_ui_anim_control_scroll_process_y(plugin_ui_anim_control_scroll_t control_scroll) {
    return control_scroll->m_cfg_process_y;
}

static int plugin_ui_anim_control_scroll_init(plugin_ui_animation_t animation, void * ctx) {
    plugin_ui_module_t module = ctx;
    plugin_ui_anim_control_scroll_t control_scroll = plugin_ui_animation_data(animation);

    control_scroll->m_module = module;
    bzero(&control_scroll->m_cfg_percent_decorator, sizeof(control_scroll->m_cfg_percent_decorator));
    control_scroll->m_cfg_target_pos = UI_VECTOR_2_IDENTITY;
    control_scroll->m_cfg_guard_done = 1;
    control_scroll->m_cfg_take_time = 0.0f;
    control_scroll->m_cfg_process_x = 0;
    control_scroll->m_cfg_process_y = 0;
    control_scroll->m_origin_pos = UI_VECTOR_2_ZERO;
    control_scroll->m_target_pos = UI_VECTOR_2_ZERO;
    control_scroll->m_runing_time = 0.0f;
    return 0;
}

static int plugin_ui_anim_control_scroll_enter(plugin_ui_animation_t animation, void * ctx) {
    plugin_ui_module_t module = ctx;
    plugin_ui_anim_control_scroll_t control_scroll = plugin_ui_animation_data(animation);
    plugin_ui_control_t control;
    
    control_scroll->m_runing_time = 0.0f;
    
    if (control_scroll->m_cfg_take_time == 0.0f) {
        CPE_ERROR(module->m_em, "plugin_ui_anim_control_scroll_enter: take-time not configured!");
        return -1;
    }
    
    control = plugin_ui_animation_find_first_tie_control(animation);
    if (control == NULL) return -1;

    control_scroll->m_origin_pos = *plugin_ui_control_all_frame_pos(control);
    control_scroll->m_target_pos = control_scroll->m_cfg_target_pos;

    return 0;
}

static void plugin_ui_anim_control_scroll_exit(plugin_ui_animation_t animation, void * ctx) {
    plugin_ui_anim_control_scroll_t control_scroll = plugin_ui_animation_data(animation);
    plugin_ui_control_t control;

    if (control_scroll->m_cfg_guard_done) {
        control = plugin_ui_animation_find_first_tie_control(animation);
        if (control == NULL) return;

        if (control_scroll->m_cfg_process_x) {
            plugin_ui_control_set_scroll_x(control, control_scroll->m_target_pos.x);
        }

        if (control_scroll->m_cfg_process_y) {
            plugin_ui_control_set_scroll_y(control, control_scroll->m_target_pos.y);
        }
    }
}

static uint8_t plugin_ui_anim_control_scroll_update(plugin_ui_animation_t animation, void * ctx, float delta) {
    plugin_ui_anim_control_scroll_t control_scroll = plugin_ui_animation_data(animation);
    plugin_ui_control_t control;
    float percent;
    
    control = plugin_ui_animation_find_first_tie_control(animation);
    if (control == NULL) return 0;

    control_scroll->m_runing_time += delta;

    if (control_scroll->m_runing_time >= control_scroll->m_cfg_take_time) {
        percent = 1.0f;
    }
    else {
        percent = control_scroll->m_runing_time / control_scroll->m_cfg_take_time;
    }

    percent = ui_percent_decorator_decorate(&control_scroll->m_cfg_percent_decorator, percent);

    if (control_scroll->m_cfg_process_x) {
        float x = control_scroll->m_origin_pos.x + (control_scroll->m_target_pos.x - control_scroll->m_origin_pos.x) * percent;
        plugin_ui_control_set_scroll_x(control, x);
    }

    if (control_scroll->m_cfg_process_y) {
        float y = control_scroll->m_origin_pos.y + (control_scroll->m_target_pos.y - control_scroll->m_origin_pos.y) * percent;
        plugin_ui_control_set_scroll_y(control, y);
    }

    return control_scroll->m_runing_time >= control_scroll->m_cfg_take_time ? 0 : 1;
}

static int plugin_ui_anim_control_scroll_setup(
    plugin_ui_animation_t animation, void * ctx, char * arg_buf_will_change,
    plugin_ui_control_t control, plugin_ui_control_frame_t frame)
{
    plugin_ui_module_t module = ctx;
    plugin_ui_anim_control_scroll_t control_scroll = plugin_ui_animation_data(animation);
    char * str_value;

    if (plugin_ui_animation_control_create(animation, control, 1) == NULL) {
        CPE_ERROR(module->m_em, "plugin_ui_anim_control_scroll_setup: create animation control fail!");
        return -1;
    }

    if((str_value = cpe_str_read_and_remove_arg(arg_buf_will_change, "control-scroll.take-time", ',', '='))) {
        control_scroll->m_cfg_take_time = atof(str_value);
    }
    else if((str_value = cpe_str_read_and_remove_arg(arg_buf_will_change, "control-scroll.take-time-frame", ',', '='))) {
        control_scroll->m_cfg_take_time = atof(str_value) / module->m_cfg_fps;
    }
    else {
        CPE_ERROR(module->m_em, "plugin_ui_anim_control_scroll_setup: setup: take-time not configured!");
        return -1;
    }
    
    if((str_value = cpe_str_read_and_remove_arg(arg_buf_will_change, "control-scroll.decorator", ',', '='))) {
        if (ui_percent_decorator_setup(&control_scroll->m_cfg_percent_decorator, str_value, module->m_em) != 0) {
            CPE_ERROR(
                module->m_em, "%s: control-move-in: create: set decorator %s fail!",
                plugin_ui_module_name(module), str_value);
            return -1;
        }
    }

    if((str_value = cpe_str_read_and_remove_arg(arg_buf_will_change, "control-scroll.target-pos", ',', '='))) {
        control_scroll->m_cfg_target_pos.x = control_scroll->m_cfg_target_pos.y = atof(str_value);
        control_scroll->m_cfg_process_x = control_scroll->m_cfg_process_y = 1;
    }

    if((str_value = cpe_str_read_and_remove_arg(arg_buf_will_change, "control-scroll.target-pos.x", ',', '='))) {
        control_scroll->m_cfg_target_pos.x = atof(str_value);
        control_scroll->m_cfg_process_x = 1;
    }

    if((str_value = cpe_str_read_and_remove_arg(arg_buf_will_change, "control-scroll.target-pos.y", ',', '='))) {
        control_scroll->m_cfg_target_pos.y = atof(str_value);
        control_scroll->m_cfg_process_y = 1;
    }

    return 0;
}

int plugin_ui_anim_control_scroll_regist(plugin_ui_module_t module) {
    plugin_ui_animation_meta_t meta =
        plugin_ui_animation_meta_create(
            module, PLUGIN_UI_ANIM_CONTROL_SCROLL, module,
            /*animation*/
            sizeof(struct plugin_ui_anim_control_scroll),
            plugin_ui_anim_control_scroll_init,
            NULL,
            plugin_ui_anim_control_scroll_enter,
            plugin_ui_anim_control_scroll_exit,
            plugin_ui_anim_control_scroll_update,
            /*control*/
            0, NULL, NULL,
            /*setup*/
            plugin_ui_anim_control_scroll_setup);
    return meta ? 0 : -1;
}

void plugin_ui_anim_control_scroll_unregist(plugin_ui_module_t module) {
    plugin_ui_animation_meta_t meta = plugin_ui_animation_meta_find(module, PLUGIN_UI_ANIM_CONTROL_SCROLL);
    if (meta) {
        plugin_ui_animation_meta_free(meta);
    }
}

const char * PLUGIN_UI_ANIM_CONTROL_SCROLL = "control-scroll";

