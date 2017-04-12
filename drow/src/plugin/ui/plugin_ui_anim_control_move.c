#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/math_ex.h"
#include "cpe/utils/string_utils.h"
#include "plugin/ui/plugin_ui_animation_meta.h"
#include "plugin/ui/plugin_ui_animation_control.h"
#include "plugin/ui/plugin_ui_move_algorithm.h"
#include "plugin/ui/plugin_ui_aspect.h"
#include "plugin_ui_anim_control_move_i.h"
#include "plugin_ui_env_i.h"
#include "plugin_ui_control_i.h"
#include "plugin_ui_page_i.h"
#include "plugin_ui_animation_i.h"
#include "plugin_ui_move_algorithm_i.h"
#include "plugin_ui_move_algorithm_meta_i.h"

static int plugin_ui_anim_control_move_setup(
    plugin_ui_animation_t animation, void * ctx, char * arg_buf_will_change,
    plugin_ui_control_t control, plugin_ui_control_frame_t frame);
static uint8_t plugin_ui_anim_control_move_update(plugin_ui_animation_t animation, void * ctx, float delta_s);

plugin_ui_anim_control_move_t
plugin_ui_anim_control_move_create(plugin_ui_control_t control, char * arg_buf_will_change) {
    plugin_ui_env_t env = control->m_page->m_env;
    plugin_ui_animation_t animation;
    
    animation = plugin_ui_animation_create_by_type_name(env, PLUGIN_UI_ANIM_CONTROL_MOVE);
    if (animation == NULL) return NULL;

    if (plugin_ui_anim_control_move_setup(animation, env->m_module, arg_buf_will_change, control, NULL)) {
        plugin_ui_animation_free(animation);
        return NULL;
    }

    return plugin_ui_animation_data(animation);
}

int plugin_ui_anim_control_move_set_decorator(plugin_ui_anim_control_move_t move, const char * decorator) {
    if (ui_percent_decorator_setup(&move->m_cfg_percent_decorator, decorator, move->m_module->m_em) != 0) {
        CPE_ERROR(
            move->m_module->m_em, "%s: control-move: set decorator %s fail!",
            plugin_ui_module_name(move->m_module), decorator);
        return -1;
    }

    return 0;
}

void plugin_ui_anim_control_move_set_take_time(plugin_ui_anim_control_move_t move, float take_time) {
    move->m_cfg_take_time = take_time;
}

void plugin_ui_anim_control_move_set_take_time_frame(plugin_ui_anim_control_move_t move, uint32_t take_time_frame) {
    move->m_cfg_take_time = (float)take_time_frame / move->m_module->m_cfg_fps;
}

static void plugin_ui_anim_control_move_on_algorithm_fini(void * user_ctx, plugin_ui_move_algorithm_t algorithm) {
    plugin_ui_anim_control_move_t control_move = user_ctx;
    assert(control_move->m_cfg_algorithm == algorithm);
    control_move->m_cfg_algorithm = NULL;
}

void plugin_ui_anim_control_move_set_algorithm(plugin_ui_anim_control_move_t move, plugin_ui_move_algorithm_t algorithm) {
    if (move->m_cfg_algorithm) {
        move->m_cfg_algorithm->m_user_on_fini = NULL;
        move->m_cfg_algorithm->m_user_ctx = NULL;
        plugin_ui_move_algorithm_free(move->m_cfg_algorithm);
    }
    
    move->m_cfg_algorithm = algorithm;
    
    if (move->m_cfg_algorithm) {
        move->m_cfg_algorithm->m_user_on_fini = plugin_ui_anim_control_move_on_algorithm_fini;
        move->m_cfg_algorithm->m_user_ctx = move;
    }
}

int plugin_ui_anim_control_move_set_origin(plugin_ui_anim_control_move_t move, const char * origin) {
    if (move->m_cfg_origin) {
        mem_free(move->m_module->m_alloc, move->m_cfg_origin);
    }

    if (origin) {
        move->m_cfg_origin = cpe_str_mem_dup_trim(move->m_module->m_alloc, origin);
        if (move->m_cfg_origin == NULL) {
            CPE_ERROR(
                move->m_module->m_em, "%s: control-move: set origin %s alloc fail!",
                plugin_ui_module_name(move->m_module), origin);
            return -1;
        }
    }
    else {
        move->m_cfg_origin = NULL;
    }

    return 0;
}

int plugin_ui_anim_control_move_set_target(plugin_ui_anim_control_move_t move, const char * target) {
    if (move->m_cfg_target) {
        mem_free(move->m_module->m_alloc, move->m_cfg_target);
    }

    if (target) {
        move->m_cfg_target = cpe_str_mem_dup_trim(move->m_module->m_alloc, target);
        if (move->m_cfg_target == NULL) {
            CPE_ERROR(
                move->m_module->m_em, "%s: control-move: set target %s alloc fail!",
                plugin_ui_module_name(move->m_module), target);
            return -1;
        }
    }
    else {
        move->m_cfg_target = NULL;
    }

    return 0;
}

static int plugin_ui_anim_control_move_init(plugin_ui_animation_t animation, void * ctx) {
    plugin_ui_anim_control_move_t control_move = plugin_ui_animation_data(animation);

    control_move->m_module = ctx;
    bzero(&control_move->m_cfg_percent_decorator, sizeof(control_move->m_cfg_percent_decorator));
    control_move->m_cfg_algorithm = NULL;
    control_move->m_cfg_take_time = 0.0f;
    control_move->m_cfg_origin = NULL;
    control_move->m_cfg_target = NULL;
    control_move->m_cfg_update_target = 0;
    control_move->m_runing_time = 0.0f;
    control_move->m_origin = UI_VECTOR_2_ZERO;
    control_move->m_target = UI_VECTOR_2_ZERO;

    return 0;
}

static void plugin_ui_anim_control_move_free(plugin_ui_animation_t animation, void * ctx) {
    plugin_ui_module_t module = ctx;
    plugin_ui_anim_control_move_t control_move = plugin_ui_animation_data(animation);

    if (control_move->m_cfg_algorithm) {
        control_move->m_cfg_algorithm->m_user_on_fini = NULL;
        plugin_ui_move_algorithm_free(control_move->m_cfg_algorithm);
        control_move->m_cfg_algorithm = NULL;
    }

    if (control_move->m_cfg_origin) {
        mem_free(module->m_alloc, control_move->m_cfg_origin);
        control_move->m_cfg_origin = NULL;
    }
    
    if (control_move->m_cfg_target) {
        mem_free(module->m_alloc, control_move->m_cfg_target);
        control_move->m_cfg_target = NULL;
    }
}

static int plugin_ui_anim_control_move_enter(plugin_ui_animation_t animation, void * ctx) {
    plugin_ui_module_t module = ctx;
    plugin_ui_anim_control_move_t control_move = plugin_ui_animation_data(animation);
    plugin_ui_control_t control;

    if (control_move->m_cfg_algorithm == NULL) {
        plugin_ui_anim_control_move_set_algorithm(control_move, plugin_ui_move_algorithm_create_by_type_name(animation->m_env, "linear"));
        if (control_move->m_cfg_algorithm == NULL) {
            CPE_ERROR(module->m_em, "plugin_ui_anim_control_move_enter: no move algorithm!");
            return -1;
        }
    }

    if (control_move->m_cfg_origin) {
        if (plugin_ui_env_calc_world_pos(&control_move->m_origin, animation->m_env, control_move->m_cfg_origin) != 0) {
            CPE_ERROR(module->m_em, "plugin_ui_anim_control_move_setup: move algorith calc origin from %s fail!", control_move->m_cfg_origin);
            return -1;
        }
    }
    else {
        control = plugin_ui_animation_find_first_tie_control(animation);
        if (control == NULL) return -1;
        control_move->m_origin = *plugin_ui_control_real_pt_abs(control);
    }
    
    if (control_move->m_cfg_target == NULL) {
        CPE_ERROR(module->m_em, "plugin_ui_anim_control_move_enter: no target setup!");
        return -1;
    }

    if (plugin_ui_env_calc_world_pos(&control_move->m_target, animation->m_env, control_move->m_cfg_target) != 0) {
        CPE_ERROR(module->m_em, "plugin_ui_anim_control_move_enter: calc target pos from %s fail!", control_move->m_cfg_target);
        return -1;
    }

    /* printf("xxxxxx: origin=(%f,%f), target=(%f,%f)\n", */
    /*        control_move->m_origin.x, control_move->m_origin.y, */
    /*        control_move->m_target.x, control_move->m_target.y); */

    if (control_move->m_cfg_take_time == 0.0f) {
        if (control_move->m_cfg_algorithm->m_meta->m_calc_duration == NULL) {
            CPE_ERROR(
                module->m_em,
                "plugin_ui_anim_control_move_enter: algorithm %s not support calc duration!",
                control_move->m_cfg_algorithm->m_meta->m_name);
            return -1;
        }

        if (control_move->m_cfg_algorithm->m_meta->m_calc_duration(
                control_move->m_cfg_algorithm, &control_move->m_cfg_take_time, control_move->m_cfg_algorithm->m_meta->m_ctx,
                &control_move->m_origin, &control_move->m_target)
            != 0)
        {
            CPE_ERROR(
                module->m_em,
                "plugin_ui_anim_control_move_enter: algorithm %s calc duration (%f,%f) ==> (%f,%f) fail!",
                control_move->m_cfg_algorithm->m_meta->m_name,
                control_move->m_origin.x, control_move->m_origin.y, control_move->m_target.x, control_move->m_target.y);
            return -1;
        }

        if (control_move->m_cfg_take_time < 0.0f) {
            CPE_ERROR(
                module->m_em,
                "plugin_ui_anim_control_move_enter: algorithm %s calc duration (%f,%f) ==> (%f,%f) fail, result=%f!",
                control_move->m_cfg_algorithm->m_meta->m_name,
                control_move->m_origin.x, control_move->m_origin.y, control_move->m_target.x, control_move->m_target.y, control_move->m_cfg_take_time);
            return -1;
        }
    }
    
    plugin_ui_anim_control_move_update(animation, ctx, 0.0f) ;
    
    return 0;
}

static void plugin_ui_anim_control_move_exit(plugin_ui_animation_t animation, void * ctx) {
    plugin_ui_anim_control_move_t control_move = plugin_ui_animation_data(animation);
    plugin_ui_control_t control;

    control = plugin_ui_animation_find_first_tie_control(animation);
    if (control == NULL) return;

    plugin_ui_control_set_render_pt_abs(control,  &control_move->m_target);

    plugin_ui_anim_control_move_set_algorithm(control_move, NULL);
}

static uint8_t plugin_ui_anim_control_move_update(plugin_ui_animation_t animation, void * ctx, float delta_s) {
    plugin_ui_module_t module = ctx;
    plugin_ui_anim_control_move_t control_move = plugin_ui_animation_data(animation);
    plugin_ui_control_t control;
    ui_vector_2 put_pos;
    float percent;

    if (control_move->m_cfg_algorithm == NULL) return 0;

    control = plugin_ui_animation_find_first_tie_control(animation);
    if (control == NULL) return 0;

    control_move->m_runing_time += delta_s;

    assert(control_move->m_cfg_take_time > 0.0f);
    percent = control_move->m_runing_time > control_move->m_cfg_take_time ? 1.0f : (control_move->m_runing_time / control_move->m_cfg_take_time);
    assert(percent >= 0.0f && percent <= 1.0f);
    percent = ui_percent_decorator_decorate(&control_move->m_cfg_percent_decorator, percent);
    
    if (control_move->m_cfg_update_target) {
        if (plugin_ui_env_calc_world_pos(&control_move->m_target, animation->m_env, control_move->m_cfg_target) != 0) {
            CPE_ERROR(module->m_em, "plugin_ui_anim_control_move_update: calc target pos from %s fail!", control_move->m_cfg_target);
            return 0;
        }
    }

    if (control_move->m_cfg_algorithm->m_meta->m_calc_pos(
            control_move->m_cfg_algorithm, control_move->m_cfg_algorithm->m_meta->m_ctx, &put_pos,
            &control_move->m_origin, &control_move->m_target, percent)
        != 0)
    {
        CPE_ERROR(
            module->m_em,
            "plugin_ui_anim_control_move_update: algorithm %s calc pos percent=%f (%f,%f) ==> (%f,%f) fail!",
            control_move->m_cfg_algorithm->m_meta->m_name,
            percent, control_move->m_origin.x, control_move->m_origin.y, control_move->m_target.x, control_move->m_target.y);
        return 0;
    }

    /* CPE_INFO( */
    /*     module->m_em, */
    /*     "plugin_ui_anim_control_move_update: algorithm %s: (%f,%f) ==> (%f,%f), percent=%f, cur-pos=(%f,%f)!", */
    /*     control_move->m_cfg_algorithm->m_meta->m_name, */
    /*     control_move->m_origin.x, control_move->m_origin.y, control_move->m_target.x, control_move->m_target.y, */
    /*     percent, put_pos.x, put_pos.y); */
    
    plugin_ui_control_set_render_pt_abs(control, &put_pos);

    return control_move->m_runing_time > control_move->m_cfg_take_time ? 0 : 1;
}

static int plugin_ui_anim_control_move_setup(
    plugin_ui_animation_t animation, void * ctx, char * arg_buf_will_change,
    plugin_ui_control_t control, plugin_ui_control_frame_t frame)
{
    plugin_ui_module_t module = ctx;
    plugin_ui_anim_control_move_t control_move = plugin_ui_animation_data(animation);
    char * str_value;

    str_value = cpe_str_read_and_remove_arg(arg_buf_will_change, "control-move.algorithm", ',', '=');
    if (str_value) {
        plugin_ui_anim_control_move_set_algorithm(control_move, plugin_ui_move_algorithm_create_by_type_name(animation->m_env, str_value));
        if (control_move->m_cfg_algorithm == NULL) {
            CPE_ERROR(module->m_em, "plugin_ui_anim_control_move_setup: move algorith %s create fail!", str_value);
            goto SETUP_ERROR;
        }
    }

    if (control_move->m_cfg_algorithm) {
        if (plugin_ui_move_algorithm_setup(control_move->m_cfg_algorithm, arg_buf_will_change) != 0) {
            CPE_ERROR(module->m_em, "plugin_ui_anim_control_move_setup: move algorith %s setup fail!", str_value);
            goto SETUP_ERROR;
        }
    }

    str_value = cpe_str_read_and_remove_arg(arg_buf_will_change, "control-move.from", ',', '=');
    if (str_value) {
        if (plugin_ui_anim_control_move_set_origin(control_move, str_value) != 0) {
            goto SETUP_ERROR;
        }
    }
    
    str_value = cpe_str_read_and_remove_arg(arg_buf_will_change, "control-move.to", ',', '=');
    if (str_value == NULL) {
        CPE_ERROR(module->m_em, "plugin_ui_anim_control_move_setup: control-move.to not configured!");
        goto SETUP_ERROR;
    }
    else {
        if (plugin_ui_anim_control_move_set_target(control_move, str_value) != 0) {
            goto SETUP_ERROR;
        }
    }

    if ((str_value = cpe_str_read_and_remove_arg(arg_buf_will_change, "control-move.duration", ',', '='))) {
        plugin_ui_anim_control_move_set_take_time(control_move, atof(str_value));
    }
    else if ((str_value = cpe_str_read_and_remove_arg(arg_buf_will_change, "control-move.take-time", ',', '='))) {
        plugin_ui_anim_control_move_set_take_time(control_move, atof(str_value));
    }
    else if ((str_value = cpe_str_read_and_remove_arg(arg_buf_will_change, "control-move.take-time-frame", ',', '='))) {
        plugin_ui_anim_control_move_set_take_time_frame(control_move, atoi(str_value));
    }

    if ((str_value = cpe_str_read_and_remove_arg(arg_buf_will_change, "control-move.decorator", ',', '='))) {
        if (plugin_ui_anim_control_move_set_decorator(control_move, str_value) != 0) {
            goto SETUP_ERROR;
        }
    }
    
    if (plugin_ui_animation_control_create(animation, control, 1) == NULL) {
        CPE_ERROR(module->m_em, "plugin_ui_anim_control_move_setup: create animation control fail!");
        goto SETUP_ERROR;
    }

    return 0;

SETUP_ERROR:
    plugin_ui_anim_control_move_set_algorithm(control_move, NULL);
    return -1;
}

int plugin_ui_anim_control_move_regist(plugin_ui_module_t module) {
    plugin_ui_animation_meta_t meta =
        plugin_ui_animation_meta_create(
            module, PLUGIN_UI_ANIM_CONTROL_MOVE, module,
            /*animation*/
            sizeof(struct plugin_ui_anim_control_move),
            plugin_ui_anim_control_move_init,
            plugin_ui_anim_control_move_free,
            plugin_ui_anim_control_move_enter,
            plugin_ui_anim_control_move_exit,
            plugin_ui_anim_control_move_update,
            /*control*/
            0, NULL, NULL,
            /*setup*/
            plugin_ui_anim_control_move_setup);
    return meta ? 0 : -1;
}

void plugin_ui_anim_control_move_unregist(plugin_ui_module_t module) {
    plugin_ui_animation_meta_t meta = plugin_ui_animation_meta_find(module, PLUGIN_UI_ANIM_CONTROL_MOVE);
    if (meta) {
        plugin_ui_animation_meta_free(meta);
    }
}

const char * PLUGIN_UI_ANIM_CONTROL_MOVE = "control-move";
