#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/math_ex.h"
#include "cpe/utils/string_utils.h"
#include "plugin/ui/plugin_ui_animation_meta.h"
#include "plugin/ui/plugin_ui_animation_control.h"
#include "plugin/ui/plugin_ui_aspect.h"
#include "plugin_ui_anim_frame_bind_i.h"
#include "plugin_ui_env_i.h"
#include "plugin_ui_control_i.h"
#include "plugin_ui_control_frame_i.h"
#include "plugin_ui_page_i.h"
#include "plugin_ui_animation_i.h"

static int plugin_ui_anim_frame_bind_setup(
    plugin_ui_animation_t animation, void * ctx, char * arg_buf_will_change,
    plugin_ui_control_t control, plugin_ui_control_frame_t frame);

plugin_ui_anim_frame_bind_t
plugin_ui_anim_frame_bind_create(plugin_ui_control_frame_t frame, char * arg_buf_will_change) {
    plugin_ui_env_t env = frame->m_control->m_page->m_env;
    plugin_ui_animation_t animation;

    animation = plugin_ui_animation_create_by_type_name(env, PLUGIN_UI_ANIM_FRAME_BIND);
    if (animation == NULL) return NULL;

    if (plugin_ui_anim_frame_bind_setup(animation, env->m_module, arg_buf_will_change, frame->m_control, frame)) {
        plugin_ui_animation_free(animation);
        return NULL;
    }

    return plugin_ui_animation_data(animation);
}

int plugin_ui_anim_frame_bind_set_target(plugin_ui_anim_frame_bind_t frame_bind, const char * target) {
    plugin_ui_module_t module = plugin_ui_animation_from_data(frame_bind)->m_env->m_module;

    if (frame_bind->m_cfg_target) {
        mem_free(module->m_alloc, frame_bind->m_cfg_target);
    }

    if (target) {
        frame_bind->m_cfg_target = cpe_str_mem_dup(module->m_alloc, target);
        if (frame_bind->m_cfg_target == NULL) {
            CPE_ERROR(module->m_em, "plugin_ui_anim_frame_set_tar: alloc fail!");
            return -1;
        }
    }
    else {
        frame_bind->m_cfg_target = NULL;
    }

    return 0;
}

static int plugin_ui_anim_frame_bind_init(plugin_ui_animation_t animation, void * ctx) {
    plugin_ui_anim_frame_bind_t frame_bind = plugin_ui_animation_data(animation);

    frame_bind->m_cfg_target = NULL;
    frame_bind->m_remove = 0;
    
    return 0;
}

static void plugin_ui_anim_frame_bind_free(plugin_ui_animation_t animation, void * ctx) {
    plugin_ui_module_t module = ctx;
    plugin_ui_anim_frame_bind_t frame_bind = plugin_ui_animation_data(animation);

    if (frame_bind->m_cfg_target) {
        mem_free(module->m_alloc, frame_bind->m_cfg_target);
        frame_bind->m_cfg_target = NULL;
    }
}

static int plugin_ui_anim_frame_bind_enter(plugin_ui_animation_t animation, void * ctx) {
    plugin_ui_module_t module = ctx;
    plugin_ui_anim_frame_bind_t frame_bind = plugin_ui_animation_data(animation);

    if (frame_bind->m_cfg_target == NULL) {
        CPE_ERROR(module->m_em, "plugin_ui_anim_frame_bind: no target configured!");
        return -1;
    }
    
    return 0;
}

static void plugin_ui_anim_frame_bind_exit(plugin_ui_animation_t animation, void * ctx) {
    plugin_ui_anim_frame_bind_t frame_bind = plugin_ui_animation_data(animation);
    struct plugin_ui_control_frame_it frames_it;
    plugin_ui_control_frame_t frame, next_frame;

    if (frame_bind->m_remove) {
        plugin_ui_aspect_control_frames(&frames_it, plugin_ui_animation_aspect(animation));

        for(frame = plugin_ui_control_frame_it_next(&frames_it); frame; frame = next_frame) {
            next_frame = plugin_ui_control_frame_it_next(&frames_it);
            plugin_ui_control_frame_free(frame);
        }
    }
}

static uint8_t plugin_ui_anim_frame_bind_update(plugin_ui_animation_t animation, void * ctx, float delta_s) {
    plugin_ui_anim_frame_bind_t frame_bind = plugin_ui_animation_data(animation);
    struct plugin_ui_control_frame_it frames_it;
    plugin_ui_control_frame_t frame;
    uint32_t processed_count = 0;
    uint8_t have_not_exist_target = 0;
    
    plugin_ui_aspect_control_frames(&frames_it, plugin_ui_animation_aspect(animation));
    while((frame = plugin_ui_control_frame_it_next(&frames_it))) {
        plugin_ui_control_frame_t target;
        ui_vector_2 put_pos;

        processed_count++;
        
        target = plugin_ui_control_frame_find_by_name(frame->m_control, frame_bind->m_cfg_target);
        if (target == NULL) {
            have_not_exist_target++;
            continue;
        }

        put_pos = plugin_ui_control_frame_world_pos(target);
        plugin_ui_control_frame_set_world_pos(frame, &put_pos);
    }

    if (have_not_exist_target) {
        return 0;
    }
    
    if (processed_count == 0) return 0;
    return 1;
}

static int plugin_ui_anim_frame_bind_setup(
    plugin_ui_animation_t animation, void * ctx, char * arg_buf_will_change,
    plugin_ui_control_t control, plugin_ui_control_frame_t frame)
{
    plugin_ui_module_t module = ctx;
    plugin_ui_anim_frame_bind_t frame_bind = plugin_ui_animation_data(animation);
    char * str_value;
    plugin_ui_aspect_t aspect = NULL;
    
    if (frame == NULL) {
        CPE_ERROR(module->m_em, "plugin_ui_anim_frame_bind_setup: frame-bind only support work on frame!");
        return -1;
    }

    if ((str_value = cpe_str_read_and_remove_arg(arg_buf_will_change, "frame-bind.target", ',', '='))) {
        plugin_ui_anim_frame_bind_set_target(frame_bind, str_value);
    }

    if ((str_value = cpe_str_read_and_remove_arg(arg_buf_will_change, "frame-bind.on-complete", ',', '='))) {
        if (strcmp(str_value, "noop") == 0) {
            frame_bind->m_remove = 0;
        }
        else if (strcmp(str_value, "remove") == 0) {
            frame_bind->m_remove = 1;
        }
        else {
            CPE_ERROR(module->m_em, "plugin_ui_anim_frame_move_setup: frame-move.on-complete %s unknown!", str_value);
            goto SETUP_ERROR;
        }
    }

    aspect = plugin_ui_animation_aspect_check_create(animation);
    if (aspect == NULL) {
        CPE_ERROR(module->m_em, "plugin_ui_anim_frame_bind_setup: create aspect fail!");
        goto SETUP_ERROR;
    }
        
    if (plugin_ui_aspect_control_frame_add(aspect, frame, 0) != 0) {
        CPE_ERROR(module->m_em, "plugin_ui_anim_frame_bind_setup: add frame fail!");
        goto SETUP_ERROR;
    }
    
    if (plugin_ui_animation_control_create(animation, frame->m_control, 1) == NULL) {
        CPE_ERROR(module->m_em, "plugin_ui_anim_frame_bind_setup: create animation control fail!");
        goto SETUP_ERROR;
    }
    
    return 0;

SETUP_ERROR:
    if (aspect) plugin_ui_aspect_clear(aspect);
    return -1;
}

int plugin_ui_anim_frame_bind_regist(plugin_ui_module_t module) {
    plugin_ui_animation_meta_t meta =
        plugin_ui_animation_meta_create(
            module, PLUGIN_UI_ANIM_FRAME_BIND, module,
            /*animation*/
            sizeof(struct plugin_ui_anim_frame_bind),
            plugin_ui_anim_frame_bind_init,
            plugin_ui_anim_frame_bind_free,
            plugin_ui_anim_frame_bind_enter,
            plugin_ui_anim_frame_bind_exit,
            plugin_ui_anim_frame_bind_update,
            /*control*/
            0, NULL, NULL,
            /*setup*/
            plugin_ui_anim_frame_bind_setup);
    return meta ? 0 : -1;
}

void plugin_ui_anim_frame_bind_unregist(plugin_ui_module_t module) {
    plugin_ui_animation_meta_t meta = plugin_ui_animation_meta_find(module, PLUGIN_UI_ANIM_FRAME_BIND);
    if (meta) {
        plugin_ui_animation_meta_free(meta);
    }
}

const char * PLUGIN_UI_ANIM_FRAME_BIND = "frame-bind";
