#include <assert.h>
#include "cpe/pal/pal_stdlib.h"
#include "cpe/utils/string_utils.h"
#include "render/runtime/ui_runtime_render_obj.h"
#include "render/runtime/ui_runtime_render_obj_ref.h"
#include "plugin/spine/plugin_spine_utils.h"
#include "plugin/ui/plugin_ui_env.h"
#include "plugin/ui/plugin_ui_aspect.h"
#include "plugin/ui/plugin_ui_animation.h"
#include "plugin/ui/plugin_ui_animation_meta.h"
#include "plugin/ui/plugin_ui_animation_control.h"
#include "plugin/ui/plugin_ui_control.h"
#include "plugin/ui/plugin_ui_control_frame.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite_ui/ui_sprite_ui_utils.h"
#include "ui/sprite_ui/ui_sprite_ui_module.h"
#include "ui/sprite_spine/ui_sprite_spine_utils.h"
#include "ui_sprite_spine_ui_anim_resize_i.h"

ui_sprite_spine_ui_anim_resize_t
ui_sprite_spine_ui_anim_resize_create(plugin_ui_env_t env) {
    plugin_ui_animation_t animation;

    animation = plugin_ui_animation_create_by_type_name(env, UI_SPRITE_SPINE_UI_ANIM_RESIZE_NAME);
    if (animation == NULL) return NULL;
    
    return plugin_ui_animation_data(animation);
}

int ui_sprite_spine_ui_anim_resize_init(plugin_ui_animation_t animation, void * ctx) {
    ui_sprite_spine_ui_module_t module = ctx;
    ui_sprite_spine_ui_anim_resize_t anim_resize = plugin_ui_animation_data(animation);

    anim_resize->m_module = module;
    
    anim_resize->m_aspect = plugin_ui_aspect_create(plugin_ui_animation_env(animation), NULL);
    if (anim_resize->m_aspect == NULL) {
        CPE_ERROR(module->m_em, "ui_sprite_spine_ui_anim_resize_init: create asspect fail!");
        return -1;
    }

    anim_resize->m_follow = 0;
    
    return 0;
}

void ui_sprite_spine_ui_anim_resize_free(plugin_ui_animation_t animation, void * ctx) {
    ui_sprite_spine_ui_anim_resize_t anim_resize = plugin_ui_animation_data(animation);

    assert(anim_resize->m_aspect);

    plugin_ui_aspect_clear(anim_resize->m_aspect);
    plugin_ui_aspect_free(anim_resize->m_aspect);

    anim_resize->m_aspect = NULL;
}

int ui_sprite_spine_ui_anim_resize_add_frame(ui_sprite_spine_ui_anim_resize_t anim_resize, plugin_ui_control_frame_t frame) {
    plugin_ui_animation_t animation = plugin_ui_animation_from_data(anim_resize);
        
    if (plugin_ui_aspect_control_frame_add(anim_resize->m_aspect, frame, 0) != 0) {
        CPE_ERROR(anim_resize->m_module->m_em, "ui_sprite_spine_ui_anim_resize_add_frame: add to aspect fail!");
        return -1;
    }

    if (plugin_ui_animation_control_create(animation, plugin_ui_control_frame_control(frame), 1) == NULL) {
        plugin_ui_aspect_control_frame_remove(anim_resize->m_aspect, frame);
        CPE_ERROR(anim_resize->m_module->m_em, "ui_sprite_spine_ui_anim_resize_add_frame: add control to animation fail!");
        return -1;
    }

    return 0;
}

static void ui_sprite_spine_ui_anim_resize_update_frames(ui_sprite_spine_ui_module_t module, ui_sprite_spine_ui_anim_resize_t anim_resize) {
    struct plugin_ui_control_frame_it frame_it;
    plugin_ui_control_frame_t frame;

    plugin_ui_aspect_control_frames(&frame_it, anim_resize->m_aspect);

    while((frame = plugin_ui_control_frame_it_next(&frame_it))) {
        plugin_ui_control_t control = plugin_ui_control_frame_control(frame);
        ui_runtime_render_obj_ref_t render_obj_ref = plugin_ui_control_frame_render_obj_ref(frame);
        ui_runtime_render_obj_t render_obj = ui_runtime_render_obj_ref_obj(render_obj_ref);

        if (ui_runtime_render_obj_type_id(render_obj) == UI_OBJECT_TYPE_SKELETON) {
            plugin_spine_obj_t spine_obj = ui_runtime_render_obj_data(render_obj);
            ui_vector_2_t control_sz = plugin_ui_control_real_sz_no_scale(control);
            ui_vector_2_t screen_adj = plugin_ui_env_screen_adj(plugin_ui_control_env(control));
            ui_vector_2 half_size;

            half_size.x = control_sz->x / (screen_adj->x * 2.0f);
            half_size.y = control_sz->y / (screen_adj->y * 2.0f);

            if(half_size.x <= 0 || half_size.y <= 0) return;

            plugin_spine_obj_set_ik_by_name(spine_obj, "left_top", - half_size.x, half_size.y);
            plugin_spine_obj_set_ik_by_name(spine_obj, "left_down", - half_size.x, - half_size.y);
            plugin_spine_obj_set_ik_by_name(spine_obj, "right_top", half_size.x, half_size.y);
            plugin_spine_obj_set_ik_by_name(spine_obj, "right_down", half_size.x, - half_size.y);
        }
    }
}

int ui_sprite_spine_ui_anim_resize_enter(plugin_ui_animation_t animation, void * ctx) {
    ui_sprite_spine_ui_module_t module = ctx;
    ui_sprite_spine_ui_anim_resize_t anim_resize = plugin_ui_animation_data(animation);
    ui_sprite_spine_ui_anim_resize_update_frames(module, anim_resize);
    return 0;
}

void ui_sprite_spine_ui_anim_resize_exit(plugin_ui_animation_t animation, void * ctx) {
}

uint8_t ui_sprite_spine_ui_anim_resize_update(plugin_ui_animation_t animation, void * ctx, float delta_s) {
    ui_sprite_spine_ui_module_t module = ctx;
    ui_sprite_spine_ui_anim_resize_t anim_resize = plugin_ui_animation_data(animation);
    if (anim_resize->m_follow) {
        ui_sprite_spine_ui_anim_resize_update_frames(module, anim_resize);
        return 1;
    }
    else {
        return 0;
    }
}

int ui_sprite_spine_ui_anim_resize_setup(
    plugin_ui_animation_t animation, void * ctx, char * arg_buf_will_change,
    plugin_ui_control_t control, plugin_ui_control_frame_t frame)
{
    ui_sprite_spine_ui_anim_resize_t anim_resize = plugin_ui_animation_data(animation);
    const char * str_value;

    if (frame == NULL) {
        CPE_ERROR(anim_resize->m_module->m_em, "ui_sprite_spine_ui_anim_resize_setup: spine-ui-resize only support work on frame!");
        return -1;
    }

    if ((str_value = cpe_str_read_and_remove_arg(arg_buf_will_change, "spine-ui-resize-follow", ',', '='))) {
        anim_resize->m_follow = atoi(str_value);
    }
    
    return ui_sprite_spine_ui_anim_resize_add_frame(anim_resize, frame);
}

int ui_sprite_spine_ui_anim_resize_regist(ui_sprite_spine_ui_module_t module) {
    plugin_ui_animation_meta_t meta =
        plugin_ui_animation_meta_create(
            ui_sprite_ui_module_ui_module(module->m_ui_module),
            UI_SPRITE_SPINE_UI_ANIM_RESIZE_NAME, module,
            /*animation*/
            sizeof(struct ui_sprite_spine_ui_anim_resize),
            ui_sprite_spine_ui_anim_resize_init,
            ui_sprite_spine_ui_anim_resize_free,
            ui_sprite_spine_ui_anim_resize_enter,
            ui_sprite_spine_ui_anim_resize_exit,
            ui_sprite_spine_ui_anim_resize_update,
            /*control*/
            0, NULL, NULL,
            ui_sprite_spine_ui_anim_resize_setup);
    
    return meta ? 0 : -1;
}

void ui_sprite_spine_ui_anim_resize_unregist(ui_sprite_spine_ui_module_t module) {
    plugin_ui_animation_meta_t meta =
        plugin_ui_animation_meta_find(
            ui_sprite_ui_module_ui_module(module->m_ui_module),
            UI_SPRITE_SPINE_UI_ANIM_RESIZE_NAME);
    if (meta) {
        plugin_ui_animation_meta_free(meta);
    }
}

const char * UI_SPRITE_SPINE_UI_ANIM_RESIZE_NAME = "spine-ui-resize";
