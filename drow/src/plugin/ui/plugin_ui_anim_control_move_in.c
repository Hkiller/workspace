#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/math_ex.h"
#include "cpe/utils/string_utils.h"
#include "render/model/ui_data_layout.h"
#include "plugin/ui/plugin_ui_animation.h"
#include "plugin/ui/plugin_ui_animation_meta.h"
#include "plugin/ui/plugin_ui_animation_control.h"
#include "plugin_ui_anim_control_move_in_i.h"
#include "plugin_ui_env_i.h"
#include "plugin_ui_control_i.h"
#include "plugin_ui_page_i.h"
#include "plugin_ui_utils_i.h"

static int plugin_ui_anim_control_move_in_setup(
    plugin_ui_animation_t animation, void * ctx, char * arg_buf_will_change,
    plugin_ui_control_t control, plugin_ui_control_frame_t frame);

plugin_ui_anim_control_move_in_t
plugin_ui_anim_control_move_in_create(plugin_ui_control_t control) {
    plugin_ui_env_t env = control->m_page->m_env;
    plugin_ui_animation_t animation;

    animation = plugin_ui_animation_create_by_type_name(env, PLUGIN_UI_ANIM_CONTROL_MOVE_IN);
    if (animation == NULL) return NULL;

    return plugin_ui_animation_data(animation);
}

plugin_ui_anim_control_move_in_t
plugin_ui_anim_control_move_in_create_with_setup(plugin_ui_control_t control, char * arg_buf_will_change) {
    plugin_ui_env_t env = control->m_page->m_env;
    plugin_ui_animation_t animation;

    animation = plugin_ui_animation_create_by_type_name(env, PLUGIN_UI_ANIM_CONTROL_MOVE_IN);
    if (animation == NULL) return NULL;

    if (plugin_ui_anim_control_move_in_setup(animation, env->m_module, arg_buf_will_change, control, NULL)) {
        plugin_ui_animation_free(animation);
        return NULL;
    }

    return plugin_ui_animation_data(animation);
}

int plugin_ui_anim_control_move_in_set_decorator(plugin_ui_anim_control_move_in_t move_in, const char * decorator) {
    if (ui_percent_decorator_setup(&move_in->m_cfg_percent_decorator, decorator, move_in->m_module->m_em) != 0) {
        CPE_ERROR(
            move_in->m_module->m_em, "%s: control-move-in: set decorator %s fail!",
            plugin_ui_module_name(move_in->m_module), decorator);
        return -1;
    }

    return 0;
}

void plugin_ui_anim_control_move_in_set_take_time(plugin_ui_anim_control_move_in_t move_in, float take_time) {
    move_in->m_cfg_take_time = take_time;
}

void plugin_ui_anim_control_move_in_set_take_time_frame(plugin_ui_anim_control_move_in_t move_in, uint32_t take_time_frame) {
    move_in->m_cfg_take_time = (float)take_time_frame / move_in->m_module->m_cfg_fps;
}

void plugin_ui_anim_control_move_in_set_origin_pos(plugin_ui_anim_control_move_in_t move_in, plugin_ui_control_move_pos_t pos) {
    move_in->m_cfg_origin = pos;
}

void plugin_ui_anim_control_move_in_set_begin_at(plugin_ui_anim_control_move_in_t move_in, float begin_at) {
    move_in->m_cfg_begin_at = begin_at;
}

static int plugin_ui_anim_control_move_in_init(plugin_ui_animation_t animation, void * ctx) {
    plugin_ui_module_t module = ctx;
    plugin_ui_anim_control_move_in_t control_move_in = plugin_ui_animation_data(animation);

    control_move_in->m_module = module;
    bzero(&control_move_in->m_cfg_percent_decorator, sizeof(control_move_in->m_cfg_percent_decorator));
    control_move_in->m_cfg_origin = plugin_ui_control_move_left;
    control_move_in->m_cfg_take_time = 0.0f;
    control_move_in->m_origin = UI_VECTOR_2_ZERO;
    control_move_in->m_target = UI_VECTOR_2_ZERO;
    control_move_in->m_runing_time = 0.0f;
	control_move_in->m_cfg_begin_at = 0.0f;
    return 0;
}

static int plugin_ui_anim_control_move_in_enter(plugin_ui_animation_t animation, void * ctx) {
    plugin_ui_module_t module = ctx;
    plugin_ui_anim_control_move_in_t control_move_in = plugin_ui_animation_data(animation);
    ui_vector_2_t parent_sz;    
    ui_vector_2 self_pt;
    ui_vector_2_t self_sz;
    plugin_ui_control_t control;

    control_move_in->m_runing_time = 0.0f;

    if (control_move_in->m_cfg_take_time == 0.0f) {
        CPE_ERROR(module->m_em, "plugin_ui_anim_control_move_in_enter: take-time not configured!");
        return -1;
    }
    
    control = plugin_ui_animation_find_first_tie_control(animation);
    if (control == NULL) return -1;
    
    if (plugin_ui_control_need_update_cache(control)) {
        plugin_ui_control_update_cache(control, 0);
    }

    parent_sz = plugin_ui_control_real_sz_no_scale(plugin_ui_control_parent(control));
    self_pt = *plugin_ui_control_real_pt_to_p(control);
    self_sz = plugin_ui_control_real_sz_no_scale(control);

    switch(plugin_ui_control_align_horz(control)) {
    case ui_align_mode_horz_left:
        control_move_in->m_target.x = 0;
        break;
    case ui_align_mode_horz_right:
        control_move_in->m_target.x = parent_sz->x - self_sz->x;
        break;
    case ui_align_mode_horz_center:
        control_move_in->m_target.x = (parent_sz->x - self_sz->x) / 2.0f;
        break;
    default:
        control_move_in->m_target.x = self_pt.x;
        break;
    }

    switch(plugin_ui_control_align_vert(control)) {
    case ui_align_mode_vert_top:
        control_move_in->m_target.y = 0;
        break;
    case ui_align_mode_vert_bottom:
        control_move_in->m_target.y = parent_sz->y - self_sz->y;
        break;
    case ui_align_mode_vert_center:
        control_move_in->m_target.y = (parent_sz->y - self_sz->y) / 2.0f;
        break;
    default:
        control_move_in->m_target.y = self_pt.y;
        break;
    }

	switch(control_move_in->m_cfg_origin) {
    case plugin_ui_control_move_left:
		control_move_in->m_origin.x = -1 * self_sz->x;
		control_move_in->m_origin.y = control_move_in->m_target.y;
        break;
    case plugin_ui_control_move_right:
		control_move_in->m_origin.x = parent_sz->x;
		control_move_in->m_origin.y = control_move_in->m_target.y;
        break;
    case plugin_ui_control_move_top:
		control_move_in->m_origin.x = control_move_in->m_target.x;
		control_move_in->m_origin.y = -1 * self_sz->y;
        break;
    case plugin_ui_control_move_bottom:
		control_move_in->m_origin.x = control_move_in->m_target.x;
		control_move_in->m_origin.y = parent_sz->y;
        break;
    case plugin_ui_control_move_left_top:
        control_move_in->m_origin.x = -1 * self_sz->x;
        control_move_in->m_origin.y = -1 * self_sz->y;
        break;
    case plugin_ui_control_move_left_bottom:
        control_move_in->m_origin.x = -1 * self_sz->x;
        control_move_in->m_origin.y = parent_sz->y;
        break;
    case plugin_ui_control_move_right_top:
        control_move_in->m_origin.x = parent_sz->x;
        control_move_in->m_origin.y = -1 * self_sz->y;
        break;
    case plugin_ui_control_move_right_bottom:
        control_move_in->m_origin.x = parent_sz->x;
        control_move_in->m_origin.y = parent_sz->y;
        break;
    default:
        CPE_ERROR(module->m_em, "plugin_ui_anim_control_move_in_enter: origin pos %d unknown!", control_move_in->m_cfg_origin);
		return -1;
    }

	if (control_move_in->m_cfg_begin_at != 0.0f) {
		control_move_in->m_origin.x = control_move_in->m_origin.x + (control_move_in->m_target.x - control_move_in->m_origin.x) * control_move_in->m_cfg_begin_at;
		control_move_in->m_origin.y = control_move_in->m_origin.y + (control_move_in->m_target.y - control_move_in->m_origin.y) * control_move_in->m_cfg_begin_at;
	}

	plugin_ui_control_set_render_pt_by_real(control, &control_move_in->m_origin);

    plugin_ui_control_set_visible(control, 1);

    return 0;
}

static void plugin_ui_anim_control_move_in_exit(plugin_ui_animation_t animation, void * ctx) {
    plugin_ui_anim_control_move_in_t control_move_in = plugin_ui_animation_data(animation);
    plugin_ui_control_t control;

    control = plugin_ui_animation_find_first_tie_control(animation);
    if (control == NULL) return;

    plugin_ui_control_set_render_pt_by_real(control, &control_move_in->m_target);
}

static uint8_t plugin_ui_anim_control_move_in_update(plugin_ui_animation_t animation, void * ctx, float delta) {
    plugin_ui_anim_control_move_in_t control_move_in = plugin_ui_animation_data(animation);
    plugin_ui_control_t control;
    float percent;
    ui_vector_2 cur_pos;

    control = plugin_ui_animation_find_first_tie_control(animation);
    if (control == NULL) return 0;

    control_move_in->m_runing_time += delta;

    if (control_move_in->m_runing_time >= control_move_in->m_cfg_take_time) {
        percent = 1.0f;
    }
    else {
        percent = control_move_in->m_runing_time / control_move_in->m_cfg_take_time;
    }

    percent = ui_percent_decorator_decorate(&control_move_in->m_cfg_percent_decorator, percent);

	cur_pos.x = control_move_in->m_origin.x + (control_move_in->m_target.x - control_move_in->m_origin.x) * percent;
    cur_pos.y = control_move_in->m_origin.y + (control_move_in->m_target.y - control_move_in->m_origin.y) * percent;

    plugin_ui_control_set_render_pt_by_real(control, &cur_pos);
    
    return control_move_in->m_runing_time >= control_move_in->m_cfg_take_time ? 0 : 1;
}

static int plugin_ui_anim_control_move_in_setup(
    plugin_ui_animation_t animation, void * ctx, char * arg_buf_will_change,
    plugin_ui_control_t control, plugin_ui_control_frame_t frame)
{
    plugin_ui_module_t module = ctx;
    plugin_ui_anim_control_move_in_t control_move_in = plugin_ui_animation_data(animation);
    char * str_value;

    if (plugin_ui_animation_control_create(animation, control, 1) == NULL) {
        CPE_ERROR(module->m_em, "plugin_ui_anim_control_move_in_setup: create animation control fail!");
        return -1;
    }

    if((str_value = cpe_str_read_and_remove_arg(arg_buf_will_change, "control-move-in.take-time", ',', '='))) {
        control_move_in->m_cfg_take_time = atof(str_value);
    }
    else if((str_value = cpe_str_read_and_remove_arg(arg_buf_will_change, "control-move-in.take-time-frame", ',', '='))) {
        control_move_in->m_cfg_take_time = atof(str_value) / module->m_cfg_fps;
    }
    else {
        CPE_ERROR(module->m_em, "plugin_ui_anim_control_move_in_setup: setup: take-time not configured!");
        return -1;
    }
    
	if((str_value = cpe_str_read_and_remove_arg(arg_buf_will_change, "control-move-in.begin-at", ',', '='))) {
		control_move_in->m_cfg_begin_at = atof(str_value);
	}

    if((str_value = cpe_str_read_and_remove_arg(arg_buf_will_change, "control-move-in.decorator", ',', '='))) {
        if (ui_percent_decorator_setup(&control_move_in->m_cfg_percent_decorator, str_value, module->m_em) != 0) {
            CPE_ERROR(
                module->m_em, "%s: control-move-in: create: set decorator %s fail!",
                plugin_ui_module_name(module), str_value);
            return -1;
        }
    }

    if((str_value = cpe_str_read_and_remove_arg(arg_buf_will_change, "control-move-in.origin", ',', '='))) {
        if(strcmp(str_value, "left") == 0) {
            control_move_in->m_cfg_origin = plugin_ui_control_move_left;
        }
        else if(strcmp(str_value, "right") == 0) {
            control_move_in->m_cfg_origin = plugin_ui_control_move_right;
        }
        else if(strcmp(str_value, "top") == 0) {
            control_move_in->m_cfg_origin = plugin_ui_control_move_top;
        }
        else if(strcmp(str_value, "bottom") == 0) {
            control_move_in->m_cfg_origin = plugin_ui_control_move_bottom;
        }
        else if(strcmp(str_value, "left-top") == 0) {
            control_move_in->m_cfg_origin = plugin_ui_control_move_left_top;
        }
        else if(strcmp(str_value, "left-bottom") == 0) {
            control_move_in->m_cfg_origin = plugin_ui_control_move_left_bottom;
        }
        else if(strcmp(str_value, "right-top") == 0) {
            control_move_in->m_cfg_origin = plugin_ui_control_move_right_top;
        }
        else if(strcmp(str_value, "right-bottom") == 0) {
            control_move_in->m_cfg_origin = plugin_ui_control_move_right_bottom;
        }
        else {
            CPE_ERROR(module->m_em, "%s: control-move-in: create: origin pos %s unknown!", plugin_ui_module_name(module), str_value);
            return -1;
        }
    }
    else {
        CPE_ERROR(module->m_em, "%s: control-move-in: create: origin not configured!", plugin_ui_module_name(module));
        return -1;
    }
    
    return 0;
}

int plugin_ui_anim_control_move_in_regist(plugin_ui_module_t module) {
    plugin_ui_animation_meta_t meta =
        plugin_ui_animation_meta_create(
            module, PLUGIN_UI_ANIM_CONTROL_MOVE_IN, module,
            /*animation*/
            sizeof(struct plugin_ui_anim_control_move_in),
            plugin_ui_anim_control_move_in_init,
            NULL,
            plugin_ui_anim_control_move_in_enter,
            plugin_ui_anim_control_move_in_exit,
            plugin_ui_anim_control_move_in_update,
            /*control*/
            0, NULL, NULL,
            /*setup*/
            plugin_ui_anim_control_move_in_setup);
    return meta ? 0 : -1;
}

void plugin_ui_anim_control_move_in_unregist(plugin_ui_module_t module) {
    plugin_ui_animation_meta_t meta = plugin_ui_animation_meta_find(module, PLUGIN_UI_ANIM_CONTROL_MOVE_IN);
    if (meta) {
        plugin_ui_animation_meta_free(meta);
    }
}

const char * PLUGIN_UI_ANIM_CONTROL_MOVE_IN = "control-move-in";

