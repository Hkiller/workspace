#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/math_ex.h"
#include "cpe/utils/string_utils.h"
#include "plugin/ui/plugin_ui_animation_meta.h"
#include "plugin/ui/plugin_ui_animation_control.h"
#include "plugin/ui/plugin_ui_move_algorithm.h"
#include "plugin/ui/plugin_ui_aspect.h"
#include "plugin_ui_anim_frame_move_i.h"
#include "plugin_ui_env_i.h"
#include "plugin_ui_control_i.h"
#include "plugin_ui_control_frame_i.h"
#include "plugin_ui_page_i.h"
#include "plugin_ui_animation_i.h"
#include "plugin_ui_move_algorithm_i.h"
#include "plugin_ui_move_algorithm_meta_i.h"

static int plugin_ui_anim_frame_move_setup(
    plugin_ui_animation_t animation, void * ctx, char * arg_buf_will_change,
    plugin_ui_control_t control, plugin_ui_control_frame_t frame);
static uint8_t plugin_ui_anim_frame_move_update(plugin_ui_animation_t animation, void * ctx, float delta_s);

plugin_ui_anim_frame_move_t
plugin_ui_anim_frame_move_create(plugin_ui_control_frame_t frame, char * arg_buf_will_change) {
    plugin_ui_env_t env = frame->m_control->m_page->m_env;
    plugin_ui_animation_t animation;
    
    animation = plugin_ui_animation_create_by_type_name(env, PLUGIN_UI_ANIM_FRAME_MOVE);
    if (animation == NULL) return NULL;

    if (plugin_ui_anim_frame_move_setup(animation, env->m_module, arg_buf_will_change, frame->m_control, frame)) {
        plugin_ui_animation_free(animation);
        return NULL;
    }

    return plugin_ui_animation_data(animation);
}

static int plugin_ui_anim_frame_move_init(plugin_ui_animation_t animation, void * ctx) {
    plugin_ui_anim_frame_move_t frame_move = plugin_ui_animation_data(animation);

    bzero(&frame_move->m_percent_decorator, sizeof(frame_move->m_percent_decorator));
    frame_move->m_algorithm = NULL;
    frame_move->m_duration = 0.0f;
    frame_move->m_complete_op = plugin_ui_anim_frame_move_complete_noop;
    frame_move->m_origin = UI_VECTOR_2_ZERO;
    frame_move->m_cfg_target = NULL;
    frame_move->m_runing_time = 0.0f;
    frame_move->m_update_target = 0;
    frame_move->m_target = UI_VECTOR_2_ZERO;

    return 0;
}

static void plugin_ui_anim_frame_move_free(plugin_ui_animation_t animation, void * ctx) {
    plugin_ui_module_t module = ctx;
    plugin_ui_anim_frame_move_t frame_move = plugin_ui_animation_data(animation);

    if (frame_move->m_algorithm) {
        frame_move->m_algorithm->m_user_on_fini = NULL;
        plugin_ui_move_algorithm_free(frame_move->m_algorithm);
        frame_move->m_algorithm = NULL;
    }

    if (frame_move->m_cfg_target) {
        mem_free(module->m_alloc, frame_move->m_cfg_target);
        frame_move->m_cfg_target = NULL;
    }
}

static int plugin_ui_anim_frame_move_enter(plugin_ui_animation_t animation, void * ctx) {
    plugin_ui_module_t module = ctx;
    plugin_ui_anim_frame_move_t frame_move = plugin_ui_animation_data(animation);

    if (frame_move->m_algorithm == NULL) {
        CPE_ERROR(module->m_em, "plugin_ui_anim_frame_move_enter: no move algorithm!");
        return -1;
    }

    if (frame_move->m_cfg_target == NULL) {
        CPE_ERROR(module->m_em, "plugin_ui_anim_frame_move_enter: no target setup!");
        return -1;
    }

    if (plugin_ui_env_calc_world_pos(&frame_move->m_target, animation->m_env, frame_move->m_cfg_target) != 0) {
        CPE_ERROR(module->m_em, "plugin_ui_anim_frame_move_enter: calc target pos from %s fail!", frame_move->m_cfg_target);
        return -1;
    }
    
    if (frame_move->m_duration == 0.0f) {
        if (frame_move->m_algorithm->m_meta->m_calc_duration == NULL) {
            CPE_ERROR(
                module->m_em,
                "plugin_ui_anim_frame_move_enter: algorithm %s not support calc duration!",
                frame_move->m_algorithm->m_meta->m_name);
            return -1;
        }

        if (frame_move->m_algorithm->m_meta->m_calc_duration(
                frame_move->m_algorithm, &frame_move->m_duration, frame_move->m_algorithm->m_meta->m_ctx,
                &frame_move->m_origin, &frame_move->m_target)
            != 0)
        {
            CPE_ERROR(
                module->m_em,
                "plugin_ui_anim_frame_move_enter: algorithm %s calc duration (%f,%f) ==> (%f,%f) fail!",
                frame_move->m_algorithm->m_meta->m_name,
                frame_move->m_origin.x, frame_move->m_origin.y, frame_move->m_target.x, frame_move->m_target.y);
            return -1;
        }

        if (frame_move->m_duration < 0.0f) {
            CPE_ERROR(
                module->m_em,
                "plugin_ui_anim_frame_move_enter: algorithm %s calc duration (%f,%f) ==> (%f,%f) fail, result=%f!",
                frame_move->m_algorithm->m_meta->m_name,
                frame_move->m_origin.x, frame_move->m_origin.y, frame_move->m_target.x, frame_move->m_target.y, frame_move->m_duration);
            return -1;
        }
    }
    
    plugin_ui_anim_frame_move_update(animation, ctx, 0.0f) ;
    
    return 0;
}

static void plugin_ui_anim_frame_move_exit(plugin_ui_animation_t animation, void * ctx) {
    plugin_ui_anim_frame_move_t frame_move = plugin_ui_animation_data(animation);
    struct plugin_ui_control_frame_it frames_it;
    plugin_ui_control_frame_t frame, next_frame;

    plugin_ui_aspect_control_frames(&frames_it, plugin_ui_animation_aspect(animation));
    for(frame = plugin_ui_control_frame_it_next(&frames_it); frame; frame = next_frame) {
        next_frame = plugin_ui_control_frame_it_next(&frames_it);
        switch(frame_move->m_complete_op) {
        case plugin_ui_anim_frame_move_complete_noop:
            plugin_ui_control_frame_set_world_pos(frame, &frame_move->m_target);
            break;
        case plugin_ui_anim_frame_move_complete_remove:
            plugin_ui_control_frame_free(frame);
            break;
        }
    }
}

static uint8_t plugin_ui_anim_frame_move_update(plugin_ui_animation_t animation, void * ctx, float delta_s) {
    plugin_ui_module_t module = ctx;
    plugin_ui_anim_frame_move_t frame_move = plugin_ui_animation_data(animation);
    struct plugin_ui_control_frame_it frames_it;
    plugin_ui_control_frame_t frame;
    uint32_t processed_count = 0;
    ui_vector_2 put_pos;
    float percent;

    if (frame_move->m_algorithm == NULL) return 0;
    
    frame_move->m_runing_time += delta_s;

    assert(frame_move->m_duration > 0.0f);
    percent = frame_move->m_runing_time > frame_move->m_duration ? 1.0f : (frame_move->m_runing_time / frame_move->m_duration);
    assert(percent >= 0.0f && percent <= 1.0f);
    percent = ui_percent_decorator_decorate(&frame_move->m_percent_decorator, percent);
    
    if (frame_move->m_update_target) {
        if (plugin_ui_env_calc_world_pos(&frame_move->m_target, animation->m_env, frame_move->m_cfg_target) != 0) {
            CPE_ERROR(module->m_em, "plugin_ui_anim_frame_move_update: calc target pos from %s fail!", frame_move->m_cfg_target);
            return 0;
        }
    }

    if (frame_move->m_algorithm->m_meta->m_calc_pos(
            frame_move->m_algorithm, frame_move->m_algorithm->m_meta->m_ctx, &put_pos,
            &frame_move->m_origin, &frame_move->m_target, percent)
        != 0)
    {
        CPE_ERROR(
            module->m_em,
            "plugin_ui_anim_frame_move_update: algorithm %s calc pos percent=%f (%f,%f) ==> (%f,%f) fail!",
            frame_move->m_algorithm->m_meta->m_name,
            percent, frame_move->m_origin.x, frame_move->m_origin.y, frame_move->m_target.x, frame_move->m_target.y);
        return 0;
    }

    /* CPE_INFO( */
    /*     module->m_em, */
    /*     "plugin_ui_anim_frame_move_update: algorithm %s: (%f,%f) ==> (%f,%f), percent=%f, cur-pos=(%f,%f)!", */
    /*     frame_move->m_algorithm->m_meta->m_name, */
    /*     frame_move->m_origin.x, frame_move->m_origin.y, frame_move->m_target.x, frame_move->m_target.y, */
    /*     percent, put_pos.x, put_pos.y); */
    
    plugin_ui_aspect_control_frames(&frames_it, plugin_ui_animation_aspect(animation));
    while((frame = plugin_ui_control_frame_it_next(&frames_it))) {
        plugin_ui_control_frame_set_world_pos(frame, &put_pos);
        processed_count++;
    }

    if (processed_count == 0) return 0;
    return frame_move->m_runing_time > frame_move->m_duration ? 0 : 1;
}

static void plugin_ui_anim_frame_mov_on_algorithm_fini(void * user_ctx, plugin_ui_move_algorithm_t algorithm) {
    plugin_ui_anim_frame_move_t frame_move = user_ctx;
    assert(frame_move->m_algorithm == algorithm);
    frame_move->m_algorithm = NULL;
}

static int plugin_ui_anim_frame_move_setup(
    plugin_ui_animation_t animation, void * ctx, char * arg_buf_will_change,
    plugin_ui_control_t control, plugin_ui_control_frame_t frame)
{
    plugin_ui_module_t module = ctx;
    plugin_ui_anim_frame_move_t frame_move = plugin_ui_animation_data(animation);
    char * str_value;
    plugin_ui_aspect_t aspect = NULL;
    
    if (frame == NULL) {
        CPE_ERROR(module->m_em, "plugin_ui_anim_frame_move_setup: frame-move only support work on frame!");
        return -1;
    }

    str_value = cpe_str_read_and_remove_arg(arg_buf_will_change, "frame-move.algorithm", ',', '=');
    if (str_value == NULL) str_value = "linear";

    assert(frame_move->m_algorithm == NULL);
    frame_move->m_algorithm = plugin_ui_move_algorithm_create_by_type_name(animation->m_env, str_value);
    if (frame_move->m_algorithm == NULL) {
        CPE_ERROR(module->m_em, "plugin_ui_anim_frame_move_setup: move algorith %s create fail!", str_value);
        goto SETUP_ERROR;
    }
    if (plugin_ui_move_algorithm_setup(frame_move->m_algorithm, arg_buf_will_change) != 0) {
        CPE_ERROR(module->m_em, "plugin_ui_anim_frame_move_setup: move algorith %s setup fail!", str_value);
        goto SETUP_ERROR;
    }
    
    if((str_value = cpe_str_read_and_remove_arg(arg_buf_will_change, "frame-move.from", ',', '='))) {
        if (plugin_ui_env_calc_world_pos(&frame_move->m_origin, animation->m_env, str_value) != 0) {
            CPE_ERROR(module->m_em, "plugin_ui_anim_frame_move_setup: move algorith calc origin from %s fail!", str_value);
            goto SETUP_ERROR;
        }
    }
    else {
        frame_move->m_origin = plugin_ui_control_frame_world_pos(frame);
    }

    str_value = cpe_str_read_and_remove_arg(arg_buf_will_change, "frame-move.to", ',', '=');
    if (str_value == NULL) {
        CPE_ERROR(module->m_em, "plugin_ui_anim_frame_move_setup: frame-move only support work on frame!");
        goto SETUP_ERROR;
    }
    else {
        assert(frame_move->m_cfg_target == NULL);
        frame_move->m_cfg_target = cpe_str_mem_dup_trim(module->m_alloc, str_value);
        if (frame_move->m_cfg_target == NULL) {
            CPE_ERROR(module->m_em, "plugin_ui_anim_frame_move_setup: frame-move.to dup fail!");
            goto SETUP_ERROR;
        }
    }

    if ((str_value = cpe_str_read_and_remove_arg(arg_buf_will_change, "frame-move.duration", ',', '='))) {
        frame_move->m_duration = atof(str_value);
        if (frame_move->m_duration <= 0.0f) {
            CPE_ERROR(module->m_em, "plugin_ui_anim_frame_move_setup: frame-move.duration %s format error!", str_value);
            goto SETUP_ERROR;
        }
    }

    if ((str_value = cpe_str_read_and_remove_arg(arg_buf_will_change, "frame-move.decorator", ',', '='))) {
        if (ui_percent_decorator_setup(&frame_move->m_percent_decorator, str_value, module->m_em) != 0) {
            CPE_ERROR(module->m_em, "plugin_ui_anim_frame_move_setup: frame-move.decorator %s format error!", str_value);
            goto SETUP_ERROR;
        }
    }
    
    if ((str_value = cpe_str_read_and_remove_arg(arg_buf_will_change, "frame-move.on-complete", ',', '='))) {
        if (strcmp(str_value, "noop") == 0) {
            frame_move->m_complete_op = plugin_ui_anim_frame_move_complete_noop;
        }
        else if (strcmp(str_value, "remove") == 0) {
            frame_move->m_complete_op = plugin_ui_anim_frame_move_complete_remove;
        }
        else {
            CPE_ERROR(module->m_em, "plugin_ui_anim_frame_move_setup: frame-move.on-complete %s unknown!", str_value);
            goto SETUP_ERROR;
        }
    }

    aspect = plugin_ui_animation_aspect_check_create(animation);
    if (aspect) {
        CPE_ERROR(module->m_em, "plugin_ui_anim_frame_move_setup: create aspect fail!");
        goto SETUP_ERROR;
    }
    
    if (plugin_ui_aspect_control_frame_add(aspect, frame, 0) != 0) {
        CPE_ERROR(module->m_em, "plugin_ui_anim_frame_move_setup: add frame fail!");
        goto SETUP_ERROR;
    }
    
    if (plugin_ui_animation_control_create(animation, frame->m_control, 1) == NULL) {
        CPE_ERROR(module->m_em, "plugin_ui_anim_frame_move_setup: create animation control fail!");
        goto SETUP_ERROR;
    }

    frame_move->m_algorithm->m_user_on_fini = plugin_ui_anim_frame_mov_on_algorithm_fini;
    frame_move->m_algorithm->m_user_ctx = frame_move;
    
    return 0;

SETUP_ERROR:
    if (frame_move->m_algorithm) {
        assert(frame_move->m_algorithm->m_user_on_fini == NULL);
        plugin_ui_move_algorithm_free(frame_move->m_algorithm);
        frame_move->m_algorithm = NULL;
    }

    if (aspect) plugin_ui_aspect_clear(aspect);
    
    return -1;
}

int plugin_ui_anim_frame_move_regist(plugin_ui_module_t module) {
    plugin_ui_animation_meta_t meta =
        plugin_ui_animation_meta_create(
            module, PLUGIN_UI_ANIM_FRAME_MOVE, module,
            /*animation*/
            sizeof(struct plugin_ui_anim_frame_move),
            plugin_ui_anim_frame_move_init,
            plugin_ui_anim_frame_move_free,
            plugin_ui_anim_frame_move_enter,
            plugin_ui_anim_frame_move_exit,
            plugin_ui_anim_frame_move_update,
            /*control*/
            0, NULL, NULL,
            /*setup*/
            plugin_ui_anim_frame_move_setup);
    return meta ? 0 : -1;
}

void plugin_ui_anim_frame_move_unregist(plugin_ui_module_t module) {
    plugin_ui_animation_meta_t meta = plugin_ui_animation_meta_find(module, PLUGIN_UI_ANIM_FRAME_MOVE);
    if (meta) {
        plugin_ui_animation_meta_free(meta);
    }
}

const char * PLUGIN_UI_ANIM_FRAME_MOVE = "frame-move";
