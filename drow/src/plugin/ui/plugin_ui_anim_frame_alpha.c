#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/math_ex.h"
#include "cpe/utils/string_utils.h"
#include "plugin/ui/plugin_ui_animation_meta.h"
#include "plugin/ui/plugin_ui_animation_control.h"
#include "plugin/ui/plugin_ui_aspect.h"
#include "plugin_ui_anim_frame_alpha_i.h"
#include "plugin_ui_env_i.h"
#include "plugin_ui_control_i.h"
#include "plugin_ui_control_frame_i.h"
#include "plugin_ui_page_i.h"
#include "plugin_ui_animation_i.h"

static int plugin_ui_anim_frame_alpha_setup(
    plugin_ui_animation_t animation, void * ctx, char * arg_buf_will_change,
    plugin_ui_control_t control, plugin_ui_control_frame_t frame);
static uint8_t plugin_ui_anim_frame_alpha_update(plugin_ui_animation_t animation, void * ctx, float delta_s);

plugin_ui_anim_frame_alpha_t
plugin_ui_anim_frame_alpha_create(plugin_ui_control_frame_t frame, char * arg_buf_will_change) {
    plugin_ui_env_t env = frame->m_control->m_page->m_env;
    plugin_ui_animation_t animation;
    
    animation = plugin_ui_animation_create_by_type_name(env, PLUGIN_UI_ANIM_FRAME_ALPHA);
    if (animation == NULL) return NULL;

    if (plugin_ui_anim_frame_alpha_setup(animation, env->m_module, arg_buf_will_change, frame->m_control, frame)) {
        plugin_ui_animation_free(animation);
        return NULL;
    }

    return plugin_ui_animation_data(animation);
}

static int plugin_ui_anim_frame_alpha_init(plugin_ui_animation_t animation, void * ctx) {
    plugin_ui_anim_frame_alpha_t frame_alpha = plugin_ui_animation_data(animation);

    bzero(&frame_alpha->m_percent_decorator, sizeof(frame_alpha->m_percent_decorator));
    frame_alpha->m_cfg_take_time = 0.0f;
    frame_alpha->m_cfg_target = 0.0f;
    frame_alpha->m_runing_time = 0.0f;

    return 0;
}

static void plugin_ui_anim_frame_alpha_free(plugin_ui_animation_t animation, void * ctx) {
}

static int plugin_ui_anim_frame_alpha_enter(plugin_ui_animation_t animation, void * ctx) {
    /* plugin_ui_module_t module = ctx; */
    /* plugin_ui_anim_frame_alpha_t frame_alpha = plugin_ui_animation_data(animation); */

    return 0;
}

static void plugin_ui_anim_frame_alpha_exit(plugin_ui_animation_t animation, void * ctx) {
    plugin_ui_anim_frame_alpha_t frame_alpha = plugin_ui_animation_data(animation);
    struct plugin_ui_control_frame_it frames_it;
    plugin_ui_control_frame_t frame, next_frame;

    plugin_ui_aspect_control_frames(&frames_it, plugin_ui_animation_aspect(animation));
    for(frame = plugin_ui_control_frame_it_next(&frames_it); frame; frame = next_frame) {
        next_frame = plugin_ui_control_frame_it_next(&frames_it);
    }
}

static uint8_t plugin_ui_anim_frame_alpha_update(plugin_ui_animation_t animation, void * ctx, float delta_s) {
    plugin_ui_module_t module = ctx;
    plugin_ui_anim_frame_alpha_t frame_alpha = plugin_ui_animation_data(animation);
    struct plugin_ui_control_frame_it frames_it;
    plugin_ui_control_frame_t frame;
    uint32_t processed_count = 0;
    float percent;

    frame_alpha->m_runing_time += delta_s;

    assert(frame_alpha->m_cfg_take_time > 0.0f);
    percent = frame_alpha->m_runing_time > frame_alpha->m_cfg_take_time ? 1.0f : (frame_alpha->m_runing_time / frame_alpha->m_cfg_take_time);
    assert(percent >= 0.0f && percent <= 1.0f);
    percent = ui_percent_decorator_decorate(&frame_alpha->m_percent_decorator, percent);
    
    plugin_ui_aspect_control_frames(&frames_it, plugin_ui_animation_aspect(animation));
    while((frame = plugin_ui_control_frame_it_next(&frames_it))) {
        processed_count++;
    }

    if (processed_count == 0) return 0;
    return frame_alpha->m_runing_time > frame_alpha->m_cfg_take_time ? 0 : 1;
}

static int plugin_ui_anim_frame_alpha_setup(
    plugin_ui_animation_t animation, void * ctx, char * arg_buf_will_change,
    plugin_ui_control_t control, plugin_ui_control_frame_t frame)
{
    plugin_ui_module_t module = ctx;
    plugin_ui_anim_frame_alpha_t frame_alpha = plugin_ui_animation_data(animation);
    char * str_value;
    plugin_ui_aspect_t aspect = NULL;
    
    if (frame == NULL) {
        CPE_ERROR(module->m_em, "plugin_ui_anim_frame_alpha_setup: frame-alpha only support work on frame!");
        return -1;
    }

    str_value = cpe_str_read_and_remove_arg(arg_buf_will_change, "frame-alpha.to", ',', '=');
    if (str_value == NULL) {
        CPE_ERROR(module->m_em, "plugin_ui_anim_frame_alpha_setup: frame-alpha only support work on frame!");
        goto SETUP_ERROR;
    }

    if ((str_value = cpe_str_read_and_remove_arg(arg_buf_will_change, "frame-alpha.take-time", ',', '='))) {
        frame_alpha->m_cfg_take_time = atof(str_value);
        if (frame_alpha->m_cfg_take_time <= 0.0f) {
            CPE_ERROR(module->m_em, "plugin_ui_anim_frame_alpha_setup: frame-alpha.take-time %s format error!", str_value);
            goto SETUP_ERROR;
        }
    }

    if ((str_value = cpe_str_read_and_remove_arg(arg_buf_will_change, "frame-alpha.decorator", ',', '='))) {
        if (ui_percent_decorator_setup(&frame_alpha->m_percent_decorator, str_value, module->m_em) != 0) {
            CPE_ERROR(module->m_em, "plugin_ui_anim_frame_alpha_setup: frame-alpha.decorator %s format error!", str_value);
            goto SETUP_ERROR;
        }
    }

    aspect = plugin_ui_animation_aspect_check_create(animation);
    if (aspect) {
        CPE_ERROR(module->m_em, "plugin_ui_anim_frame_alpha_setup: create aspect fail!");
        goto SETUP_ERROR;
    }
    
    if (plugin_ui_aspect_control_frame_add(aspect, frame, 0) != 0) {
        CPE_ERROR(module->m_em, "plugin_ui_anim_frame_alpha_setup: add frame fail!");
        goto SETUP_ERROR;
    }
    
    if (plugin_ui_animation_control_create(animation, frame->m_control, 1) == NULL) {
        CPE_ERROR(module->m_em, "plugin_ui_anim_frame_alpha_setup: create animation control fail!");
        goto SETUP_ERROR;
    }

    return 0;

SETUP_ERROR:
    if (aspect) plugin_ui_aspect_clear(aspect);
    
    return -1;
}

int plugin_ui_anim_frame_alpha_regist(plugin_ui_module_t module) {
    plugin_ui_animation_meta_t meta =
        plugin_ui_animation_meta_create(
            module, PLUGIN_UI_ANIM_FRAME_ALPHA, module,
            /*animation*/
            sizeof(struct plugin_ui_anim_frame_alpha),
            plugin_ui_anim_frame_alpha_init,
            plugin_ui_anim_frame_alpha_free,
            plugin_ui_anim_frame_alpha_enter,
            plugin_ui_anim_frame_alpha_exit,
            plugin_ui_anim_frame_alpha_update,
            /*control*/
            0, NULL, NULL,
            /*setup*/
            plugin_ui_anim_frame_alpha_setup);
    return meta ? 0 : -1;
}

void plugin_ui_anim_frame_alpha_unregist(plugin_ui_module_t module) {
    plugin_ui_animation_meta_t meta = plugin_ui_animation_meta_find(module, PLUGIN_UI_ANIM_FRAME_ALPHA);
    if (meta) {
        plugin_ui_animation_meta_free(meta);
    }
}

const char * PLUGIN_UI_ANIM_FRAME_ALPHA = "frame-alpha";
