#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/math_ex.h"
#include "cpe/utils/string_utils.h"
#include "render/model/ui_data_layout.h"
#include "plugin/ui/plugin_ui_animation.h"
#include "plugin/ui/plugin_ui_animation_meta.h"
#include "plugin/ui/plugin_ui_animation_control.h"
#include "plugin/ui/plugin_ui_control_frame.h"
#include "plugin_ui_anim_control_alpha_out_i.h"
#include "plugin_ui_env_i.h"
#include "plugin_ui_control_i.h"
#include "plugin_ui_page_i.h"
#include "plugin_ui_utils_i.h"

static int plugin_ui_anim_control_alpha_out_setup(
    plugin_ui_animation_t animation, void * ctx, char * arg_buf_will_change,
    plugin_ui_control_t control, plugin_ui_control_frame_t frame);

plugin_ui_anim_control_alpha_out_t
plugin_ui_anim_control_alpha_out_create(plugin_ui_control_t control) {
    plugin_ui_env_t env = control->m_page->m_env;
    plugin_ui_animation_t animation;

    animation = plugin_ui_animation_create_by_type_name(env, PLUGIN_UI_ANIM_CONTROL_ALPHA_OUT);
    if (animation == NULL) return NULL;

    return plugin_ui_animation_data(animation);
}

plugin_ui_anim_control_alpha_out_t
plugin_ui_anim_control_alpha_out_create_with_setup(plugin_ui_control_t control, char * arg_buf_will_change) {
    plugin_ui_env_t env = control->m_page->m_env;
    plugin_ui_animation_t animation;

    animation = plugin_ui_animation_create_by_type_name(env, PLUGIN_UI_ANIM_CONTROL_ALPHA_OUT);
    if (animation == NULL) return NULL;

    if (plugin_ui_anim_control_alpha_out_setup(animation, env->m_module, arg_buf_will_change, control, NULL)) {
        plugin_ui_animation_free(animation);
        return NULL;
    }

    return plugin_ui_animation_data(animation);
}

int plugin_ui_anim_control_alpha_out_set_decorator(plugin_ui_anim_control_alpha_out_t alpha_out, const char * decorator) {
    if (ui_percent_decorator_setup(&alpha_out->m_cfg_percent_decorator, decorator, alpha_out->m_module->m_em) != 0) {
        CPE_ERROR(
            alpha_out->m_module->m_em, "%s: control-alpha-out: set decorator %s fail!",
            plugin_ui_module_name(alpha_out->m_module), decorator);
        return -1;
    }

    return 0;
}

void plugin_ui_anim_control_alpha_out_set_take_time(plugin_ui_anim_control_alpha_out_t alpha_out, float take_time) {
    alpha_out->m_cfg_take_time = take_time;
}

void plugin_ui_anim_control_alpha_out_set_take_time_frame(plugin_ui_anim_control_alpha_out_t alpha_out, uint32_t take_time_frame) {
    alpha_out->m_cfg_take_time = (float)take_time_frame / alpha_out->m_module->m_cfg_fps;
}

void plugin_ui_anim_control_alpha_out_set_target(plugin_ui_anim_control_alpha_out_t alpha_out, float target) {
    alpha_out->m_cfg_target = target;
}

void plugin_ui_anim_control_alpha_out_add_layer(plugin_ui_anim_control_alpha_out_t alpha_out, plugin_ui_control_frame_layer_t layer) {
    cpe_ba_set(&alpha_out->m_cfg_frames, layer, cpe_ba_true);
}

static int plugin_ui_anim_control_alpha_out_init(plugin_ui_animation_t animation, void * ctx) {
    plugin_ui_module_t module = ctx;
    plugin_ui_anim_control_alpha_out_t control_alpha_out = plugin_ui_animation_data(animation);

    control_alpha_out->m_module = module;
    bzero(&control_alpha_out->m_cfg_percent_decorator, sizeof(control_alpha_out->m_cfg_percent_decorator));
    control_alpha_out->m_cfg_target = 0.0f;
    control_alpha_out->m_cfg_take_time = 0.0f;
    control_alpha_out->m_cfg_frames = 0.0f;
    control_alpha_out->m_origin = 0.0f;
    control_alpha_out->m_target = 0.0f;
    control_alpha_out->m_runing_time = 0.0f;
    return 0;
}


static void plugin_ui_anim_control_alpha_out_do_set_alpha(
    plugin_ui_anim_control_alpha_out_t control_alpha_out, plugin_ui_control_t control, float alpha, uint8_t visiable)
{
    if (control_alpha_out->m_cfg_frames) {
        struct plugin_ui_control_frame_it frame_it;
        plugin_ui_control_frame_t frame;
        
        plugin_ui_control_frames(control, &frame_it);
        while((frame = plugin_ui_control_frame_it_next(&frame_it))) {
            if (!cpe_ba_get(&control_alpha_out->m_cfg_frames, plugin_ui_control_frame_layer(frame))) continue;
            plugin_ui_control_frame_set_alpha(frame, alpha);
            plugin_ui_control_frame_set_visible(frame, visiable);
        }
    }
    else {
        plugin_ui_control_set_alpha(control, alpha);
        plugin_ui_control_set_visible(control, visiable);
    }
}

static int plugin_ui_anim_control_alpha_out_enter(plugin_ui_animation_t animation, void * ctx) {
    plugin_ui_module_t module = ctx;
    plugin_ui_anim_control_alpha_out_t control_alpha_out = plugin_ui_animation_data(animation);
    plugin_ui_control_t control;

    if (control_alpha_out->m_cfg_take_time == 0.0f) {
        CPE_ERROR(module->m_em, "plugin_ui_anim_control_alpha_out_enter: take-time not configured!");
        return -1;
    }
    
    control = plugin_ui_animation_find_first_tie_control(animation);
    if (control == NULL) return -1;
    
    control_alpha_out->m_origin = plugin_ui_control_alpha(control);
    control_alpha_out->m_target = control_alpha_out->m_cfg_target;

    plugin_ui_anim_control_alpha_out_do_set_alpha(control_alpha_out, control, control_alpha_out->m_origin, 1);
    
    return 0;
}

static void plugin_ui_anim_control_alpha_out_exit(plugin_ui_animation_t animation, void * ctx) {
    plugin_ui_anim_control_alpha_out_t control_alpha_out = plugin_ui_animation_data(animation);
    plugin_ui_control_t control;
    
    control = plugin_ui_animation_find_first_tie_control(animation);
    if (control == NULL) return;

    plugin_ui_anim_control_alpha_out_do_set_alpha(control_alpha_out, control, control_alpha_out->m_origin, 0);
}

static uint8_t plugin_ui_anim_control_alpha_out_update(plugin_ui_animation_t animation, void * ctx, float delta) {
    plugin_ui_anim_control_alpha_out_t control_alpha_out = plugin_ui_animation_data(animation);
    plugin_ui_control_t control;
    float percent;
    
    control = plugin_ui_animation_find_first_tie_control(animation);
    if (control == NULL) return 0;

    control_alpha_out->m_runing_time += delta;

    if (control_alpha_out->m_runing_time >= control_alpha_out->m_cfg_take_time) {
        percent = 1.0f;
    }
    else {
        percent = control_alpha_out->m_runing_time / control_alpha_out->m_cfg_take_time;
    }

    percent = ui_percent_decorator_decorate(&control_alpha_out->m_cfg_percent_decorator, percent);

    plugin_ui_anim_control_alpha_out_do_set_alpha(
        control_alpha_out, control,
        control_alpha_out->m_origin + (control_alpha_out->m_target - control_alpha_out->m_origin) * percent,
        1);
    
    return control_alpha_out->m_runing_time >= control_alpha_out->m_cfg_take_time ? 0 : 1;
}

static int plugin_ui_anim_control_alpha_out_setup(
    plugin_ui_animation_t animation, void * ctx, char * arg_buf_will_change,
    plugin_ui_control_t control, plugin_ui_control_frame_t frame)
{
    plugin_ui_module_t module = ctx;
    plugin_ui_anim_control_alpha_out_t control_alpha_out = plugin_ui_animation_data(animation);
    char * str_value;

    control_alpha_out->m_runing_time = 0.0f;
    
    if (plugin_ui_animation_control_create(animation, control, 1) == NULL) {
        CPE_ERROR(module->m_em, "plugin_ui_anim_control_alpha_out_setup: create animation control fail!");
        return -1;
    }

    if((str_value = cpe_str_read_and_remove_arg(arg_buf_will_change, "control-alpha-out.take-time", ',', '='))) {
        control_alpha_out->m_cfg_take_time = atof(str_value);
    }
    else if((str_value = cpe_str_read_and_remove_arg(arg_buf_will_change, "control-alpha-out.take-time-frame", ',', '='))) {
        control_alpha_out->m_cfg_take_time = atof(str_value) / module->m_cfg_fps;
    }
    else {
        CPE_ERROR(module->m_em, "plugin_ui_anim_control_alpha_out_setup: setup: take-time not configured!");
        return -1;
    }
    
    if((str_value = cpe_str_read_and_remove_arg(arg_buf_will_change, "control-alpha-out.decorator", ',', '='))) {
        if (ui_percent_decorator_setup(&control_alpha_out->m_cfg_percent_decorator, str_value, module->m_em) != 0) {
            CPE_ERROR(
                module->m_em, "%s: control-alpha-out: create: set decorator %s fail!",
                plugin_ui_module_name(module), str_value);
            return -1;
        }
    }

    if((str_value = cpe_str_read_and_remove_arg(arg_buf_will_change, "control-alpha-out.origin", ',', '='))) {
        control_alpha_out->m_origin = atof(str_value);
    }

    if((str_value = cpe_str_read_and_remove_arg(arg_buf_will_change, "control-alpha-out.target", ',', '='))) {
        control_alpha_out->m_target = atof(str_value);
    }
    
    return 0;
}

int plugin_ui_anim_control_alpha_out_regist(plugin_ui_module_t module) {
    plugin_ui_animation_meta_t meta =
        plugin_ui_animation_meta_create(
            module, PLUGIN_UI_ANIM_CONTROL_ALPHA_OUT, module,
            /*animation*/
            sizeof(struct plugin_ui_anim_control_alpha_out),
            plugin_ui_anim_control_alpha_out_init,
            NULL,
            plugin_ui_anim_control_alpha_out_enter,
            plugin_ui_anim_control_alpha_out_exit,
            plugin_ui_anim_control_alpha_out_update,
            /*control*/
            0, NULL, NULL,
            /*setup*/
            plugin_ui_anim_control_alpha_out_setup);
    return meta ? 0 : -1;
}

void plugin_ui_anim_control_alpha_out_unregist(plugin_ui_module_t module) {
    plugin_ui_animation_meta_t meta = plugin_ui_animation_meta_find(module, PLUGIN_UI_ANIM_CONTROL_ALPHA_OUT);
    if (meta) {
        plugin_ui_animation_meta_free(meta);
    }
}

const char * PLUGIN_UI_ANIM_CONTROL_ALPHA_OUT = "control-alpha-out";

