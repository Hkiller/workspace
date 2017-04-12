#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/math_ex.h"
#include "cpe/utils/string_utils.h"
#include "render/model/ui_data_layout.h"
#include "plugin/ui/plugin_ui_animation.h"
#include "plugin/ui/plugin_ui_animation_meta.h"
#include "plugin/ui/plugin_ui_animation_control.h"
#include "plugin_ui_anim_control_move_out_i.h"
#include "plugin_ui_env_i.h"
#include "plugin_ui_control_i.h"
#include "plugin_ui_page_i.h"
#include "plugin_ui_utils_i.h"

static int plugin_ui_anim_control_move_out_setup(
    plugin_ui_animation_t animation, void * ctx, char * arg_buf_will_change,
    plugin_ui_control_t control, plugin_ui_control_frame_t frame);

plugin_ui_anim_control_move_out_t
plugin_ui_anim_control_move_out_create(plugin_ui_control_t control) {
    plugin_ui_env_t env = control->m_page->m_env;
    plugin_ui_animation_t animation;

    animation = plugin_ui_animation_create_by_type_name(env, PLUGIN_UI_ANIM_CONTROL_MOVE_OUT);
    if (animation == NULL) return NULL;

    return plugin_ui_animation_data(animation);
}

plugin_ui_anim_control_move_out_t
plugin_ui_anim_control_move_out_create_with_setup(plugin_ui_control_t control, char * arg_buf_will_change) {
    plugin_ui_env_t env = control->m_page->m_env;
    plugin_ui_animation_t animation;

    animation = plugin_ui_animation_create_by_type_name(env, PLUGIN_UI_ANIM_CONTROL_MOVE_OUT);
    if (animation == NULL) return NULL;

    if (plugin_ui_anim_control_move_out_setup(animation, env->m_module, arg_buf_will_change, control, NULL)) {
        plugin_ui_animation_free(animation);
        return NULL;
    }

    return plugin_ui_animation_data(animation);
}

int plugin_ui_anim_control_move_out_set_decorator(plugin_ui_anim_control_move_out_t move_out, const char * decorator) {
    if (ui_percent_decorator_setup(&move_out->m_cfg_percent_decorator, decorator, move_out->m_module->m_em) != 0) {
        CPE_ERROR(
            move_out->m_module->m_em, "%s: control-move-out: set decorator %s fail!",
            plugin_ui_module_name(move_out->m_module), decorator);
        return -1;
    }

    return 0;
}

void plugin_ui_anim_control_move_out_set_take_time(plugin_ui_anim_control_move_out_t move_out, float take_time) {
    move_out->m_cfg_take_time = take_time;
}

void plugin_ui_anim_control_move_out_set_take_time_frame(plugin_ui_anim_control_move_out_t move_out, uint32_t take_time_frame) {
    move_out->m_cfg_take_time = (float)take_time_frame / move_out->m_module->m_cfg_fps;
}

void plugin_ui_anim_control_move_out_set_target_pos(plugin_ui_anim_control_move_out_t move_out, plugin_ui_control_move_pos_t pos) {
    move_out->m_cfg_target = pos;
}

static int plugin_ui_anim_control_move_out_init(plugin_ui_animation_t animation, void * ctx) {
    plugin_ui_module_t module = ctx;
    plugin_ui_anim_control_move_out_t control_move_out = plugin_ui_animation_data(animation);

    control_move_out->m_module = module;
    bzero(&control_move_out->m_cfg_percent_decorator, sizeof(control_move_out->m_cfg_percent_decorator));
    control_move_out->m_cfg_target = plugin_ui_control_move_left;
    control_move_out->m_cfg_take_time = 0.0f;
    control_move_out->m_origin = UI_VECTOR_2_ZERO;
    control_move_out->m_target = UI_VECTOR_2_ZERO;
    control_move_out->m_runing_time = 0.0f;
	control_move_out->m_cfg_end_at = 0.0f;
    return 0;
}

static int plugin_ui_anim_control_move_out_enter(plugin_ui_animation_t animation, void * ctx) {
    plugin_ui_module_t module = ctx;
    plugin_ui_anim_control_move_out_t control_move_out = plugin_ui_animation_data(animation);
    ui_vector_2_t parent_pt;
    ui_vector_2_t parent_sz;    
    ui_vector_2_t self_pt;
    ui_vector_2_t self_sz;
    plugin_ui_control_t control;

    control_move_out->m_runing_time = 0.0f;
    
    if (control_move_out->m_cfg_take_time == 0.0f) {
        CPE_ERROR(module->m_em, "plugin_ui_anim_control_move_out_enter: take-time not configured!");
        return -1;
    }
    
    control = plugin_ui_animation_find_first_tie_control(animation);
    if (control == NULL) return -1;
    
    if (plugin_ui_control_need_update_cache(control)) {
        plugin_ui_control_update_cache(control, 0);
    }

    parent_pt = plugin_ui_control_real_pt_to_p(plugin_ui_control_parent(control));
    parent_sz =  plugin_ui_control_real_sz_no_scale(plugin_ui_control_parent(control));
    self_pt = plugin_ui_control_real_pt_to_p(control);
    self_sz = plugin_ui_control_real_sz_no_scale(control);

    switch(plugin_ui_control_align_horz(control)) {
    case ui_align_mode_horz_left:
        control_move_out->m_origin.x = parent_pt->x;
        break;
    case ui_align_mode_horz_right:
        control_move_out->m_origin.x = parent_pt->x + parent_sz->x - self_sz->x;
        break;
    case ui_align_mode_horz_center:
        control_move_out->m_origin.x = (parent_sz->x - self_sz->x) / 2.0f;
        break;
    default:
        control_move_out->m_origin.x = self_pt->x;
        break;
    }

    switch(plugin_ui_control_align_vert(control)) {
    case ui_align_mode_vert_top:
        control_move_out->m_origin.y = parent_pt->y;
        break;
    case ui_align_mode_vert_bottom:
        control_move_out->m_origin.y = parent_pt->y + parent_sz->y - self_sz->y;
        break;
    case ui_align_mode_vert_center:
        control_move_out->m_origin.y = (parent_sz->y - self_sz->y) / 2.0f;
        break;
    default:
        control_move_out->m_origin.y = self_pt->y;
        break;
    }

	switch(control_move_out->m_cfg_target) {
    case plugin_ui_control_move_left:
		control_move_out->m_target.x = -1 * self_sz->x;
		control_move_out->m_target.y = control_move_out->m_origin.y;
        break;
    case plugin_ui_control_move_right:
		control_move_out->m_target.x = plugin_ui_env_runtime_sz(plugin_ui_control_env(control))->x;
		control_move_out->m_target.y = control_move_out->m_origin.y;
        break;
    case plugin_ui_control_move_top:
		control_move_out->m_target.x = control_move_out->m_origin.x;
		control_move_out->m_target.y = -1 * self_sz->y;
        break;
    case plugin_ui_control_move_bottom:
		control_move_out->m_target.x = control_move_out->m_origin.x;
		control_move_out->m_target.y = plugin_ui_env_runtime_sz(plugin_ui_control_env(control))->y;
        break;
    case plugin_ui_control_move_left_top:
        control_move_out->m_target.x = -1 * self_sz->x;
        control_move_out->m_target.y = -1 * self_sz->y;
        break;
    case plugin_ui_control_move_left_bottom:
        control_move_out->m_target.x = -1 * self_sz->x;
        control_move_out->m_target.y = plugin_ui_env_runtime_sz(plugin_ui_control_env(control))->y;
        break;
    case plugin_ui_control_move_right_top:
        control_move_out->m_target.x = plugin_ui_env_runtime_sz(plugin_ui_control_env(control))->x;
        control_move_out->m_target.y = -1 * self_sz->y;
        break;
    case plugin_ui_control_move_right_bottom:
        control_move_out->m_target.x = plugin_ui_env_runtime_sz(plugin_ui_control_env(control))->x;
        control_move_out->m_target.y = plugin_ui_env_runtime_sz(plugin_ui_control_env(control))->y;
        break;
    default:
        CPE_ERROR(module->m_em, "plugin_ui_anim_control_move_out_enter: target pos %d unknown!", control_move_out->m_cfg_target);
		return -1;
    }

	if (control_move_out->m_cfg_end_at != 0.0f) {
		control_move_out->m_target.x = control_move_out->m_origin.x + (control_move_out->m_target.x - control_move_out->m_origin.x) * control_move_out->m_cfg_end_at;
		control_move_out->m_target.y = control_move_out->m_origin.y + (control_move_out->m_target.y - control_move_out->m_origin.y) * control_move_out->m_cfg_end_at;
	}

    plugin_ui_control_set_render_pt_by_real(control, &control_move_out->m_origin);

    plugin_ui_control_set_visible(control, 1);

    return 0;
}

static void plugin_ui_anim_control_move_out_exit(plugin_ui_animation_t animation, void * ctx) {
    plugin_ui_anim_control_move_out_t control_move_out = plugin_ui_animation_data(animation);
    plugin_ui_control_t control;
    
    control = plugin_ui_animation_find_first_tie_control(animation);
    if (control == NULL) return;

    plugin_ui_control_set_render_pt_by_real(control, &control_move_out->m_origin);
    plugin_ui_control_set_visible(control, 0);
}

static uint8_t plugin_ui_anim_control_move_out_update(plugin_ui_animation_t animation, void * ctx, float delta) {
    plugin_ui_anim_control_move_out_t control_move_out = plugin_ui_animation_data(animation);
    plugin_ui_control_t control;
    float percent;
    ui_vector_2 cur_pos;
    
    control = plugin_ui_animation_find_first_tie_control(animation);
    if (control == NULL) return 0;

    control_move_out->m_runing_time += delta;

    if (control_move_out->m_runing_time >= control_move_out->m_cfg_take_time) {
        percent = 1.0f;
    }
    else {
        percent = control_move_out->m_runing_time / control_move_out->m_cfg_take_time;
    }

    percent = ui_percent_decorator_decorate(&control_move_out->m_cfg_percent_decorator, percent);

	cur_pos.x = control_move_out->m_origin.x + (control_move_out->m_target.x - control_move_out->m_origin.x) * percent;
    cur_pos.y = control_move_out->m_origin.y + (control_move_out->m_target.y - control_move_out->m_origin.y) * percent;

    plugin_ui_control_set_render_pt_by_real(control, &cur_pos);
    
    return control_move_out->m_runing_time >= control_move_out->m_cfg_take_time ? 0 : 1;
}

static int plugin_ui_anim_control_move_out_setup(
    plugin_ui_animation_t animation, void * ctx, char * arg_buf_will_change,
    plugin_ui_control_t control, plugin_ui_control_frame_t frame)
{
    plugin_ui_module_t module = ctx;
    plugin_ui_anim_control_move_out_t control_move_out = plugin_ui_animation_data(animation);
    char * str_value;

    if (plugin_ui_animation_control_create(animation, control, 1) == NULL) {
        CPE_ERROR(module->m_em, "plugin_ui_anim_control_move_out_setup: create animation control fail!");
        return -1;
    }

    if((str_value = cpe_str_read_and_remove_arg(arg_buf_will_change, "control-move-out.take-time", ',', '='))) {
        control_move_out->m_cfg_take_time = atof(str_value);
    }
    else if((str_value = cpe_str_read_and_remove_arg(arg_buf_will_change, "control-move-out.take-time-frame", ',', '='))) {
        control_move_out->m_cfg_take_time = atof(str_value) / module->m_cfg_fps;
    }
    else {
        CPE_ERROR(module->m_em, "plugin_ui_anim_control_move_out_setup: setup: take-time not configured!");
        return -1;
    }

	if((str_value = cpe_str_read_and_remove_arg(arg_buf_will_change, "control-move-out.end-at", ',', '='))) {
		control_move_out->m_cfg_end_at = atof(str_value);
	}
    
    if((str_value = cpe_str_read_and_remove_arg(arg_buf_will_change, "control-move-out.decorator", ',', '='))) {
        if (ui_percent_decorator_setup(&control_move_out->m_cfg_percent_decorator, str_value, module->m_em) != 0) {
            CPE_ERROR(
                module->m_em, "%s: control-move-out: create: set decorator %s fail!",
                plugin_ui_module_name(module), str_value);
            return -1;
        }
    }

    if((str_value = cpe_str_read_and_remove_arg(arg_buf_will_change, "control-move-out.target", ',', '='))) {
        if(strcmp(str_value, "left") == 0) {
            control_move_out->m_cfg_target = plugin_ui_control_move_left;
        }
        else if(strcmp(str_value, "right") == 0) {
            control_move_out->m_cfg_target = plugin_ui_control_move_right;
        }
        else if(strcmp(str_value, "top") == 0) {
            control_move_out->m_cfg_target = plugin_ui_control_move_top;
        }
        else if(strcmp(str_value, "bottom") == 0) {
            control_move_out->m_cfg_target = plugin_ui_control_move_bottom;
        }
        else if(strcmp(str_value, "left-top") == 0) {
            control_move_out->m_cfg_target = plugin_ui_control_move_left_top;
        }
        else if(strcmp(str_value, "left-bottom") == 0) {
            control_move_out->m_cfg_target = plugin_ui_control_move_left_bottom;
        }
        else if(strcmp(str_value, "right-top") == 0) {
            control_move_out->m_cfg_target = plugin_ui_control_move_right_top;
        }
        else if(strcmp(str_value, "right-bottom") == 0) {
            control_move_out->m_cfg_target = plugin_ui_control_move_right_bottom;
        }
        else {
            CPE_ERROR(module->m_em, "%s: control-move-out: create: target pos %s unknown!", plugin_ui_module_name(module), str_value);
            return -1;
        }
    }
    else {
        CPE_ERROR(module->m_em, "%s: control-move-out: create: target not configured!", plugin_ui_module_name(module));
        return -1;
    }
    
    return 0;
}

int plugin_ui_anim_control_move_out_regist(plugin_ui_module_t module) {
    plugin_ui_animation_meta_t meta =
        plugin_ui_animation_meta_create(
            module, PLUGIN_UI_ANIM_CONTROL_MOVE_OUT, module,
            /*animation*/
            sizeof(struct plugin_ui_anim_control_move_out),
            plugin_ui_anim_control_move_out_init,
            NULL,
            plugin_ui_anim_control_move_out_enter,
            plugin_ui_anim_control_move_out_exit,
            plugin_ui_anim_control_move_out_update,
            /*control*/
            0, NULL, NULL,
            /*setup*/
            plugin_ui_anim_control_move_out_setup);
    return meta ? 0 : -1;
}

void plugin_ui_anim_control_move_out_unregist(plugin_ui_module_t module) {
    plugin_ui_animation_meta_t meta = plugin_ui_animation_meta_find(module, PLUGIN_UI_ANIM_CONTROL_MOVE_OUT);
    if (meta) {
        plugin_ui_animation_meta_free(meta);
    }
}

const char * PLUGIN_UI_ANIM_CONTROL_MOVE_OUT = "control-move-out";

