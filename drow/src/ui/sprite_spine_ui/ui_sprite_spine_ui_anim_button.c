#include <assert.h>
#include "cpe/pal/pal_stdlib.h"
#include "cpe/utils/string_utils.h"
#include "render/model/ui_data_src.h"
#include "render/runtime/ui_runtime_render_obj.h"
#include "render/runtime/ui_runtime_render_obj_ref.h"
#include "plugin/spine/plugin_spine_utils.h"
#include "plugin/spine/plugin_spine_obj_part.h"
#include "plugin/ui/plugin_ui_env.h"
#include "plugin/ui/plugin_ui_aspect.h"
#include "plugin/ui/plugin_ui_animation.h"
#include "plugin/ui/plugin_ui_animation_meta.h"
#include "plugin/ui/plugin_ui_animation_control.h"
#include "plugin/ui/plugin_ui_control.h"
#include "plugin/ui/plugin_ui_control_action.h"
#include "plugin/ui/plugin_ui_control_frame.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite_ui/ui_sprite_ui_utils.h"
#include "ui/sprite_ui/ui_sprite_ui_module.h"
#include "ui/sprite_spine/ui_sprite_spine_utils.h"
#include "ui_sprite_spine_ui_anim_button_i.h"

ui_sprite_spine_ui_anim_button_t
ui_sprite_spine_ui_anim_button_basic_create(plugin_ui_env_t env) {
    plugin_ui_animation_t animation;

    animation = plugin_ui_animation_create_by_type_name(env, UI_SPRITE_SPINE_UI_ANIM_BUTTON_NAME);
    if (animation == NULL) return NULL;
    
    return plugin_ui_animation_data(animation);
}

int ui_sprite_spine_ui_anim_button_set_obj(ui_sprite_spine_ui_anim_button_t button, const char * obj) {
    if (button->m_obj) {
        mem_free(button->m_module->m_alloc, button->m_obj);
    }

    if (obj) {
        button->m_obj = cpe_str_mem_dup_trim(button->m_module->m_alloc, obj);
        if (button->m_obj == NULL) return -1;
    }
    else {
        button->m_obj = NULL;
    }

    return 0;
}

int ui_sprite_spine_ui_anim_button_set_part(ui_sprite_spine_ui_anim_button_t button, const char * part) {
    if (button->m_part) {
        mem_free(button->m_module->m_alloc, button->m_part);
    }

    if (part) {
        button->m_part = cpe_str_mem_dup_trim(button->m_module->m_alloc, part);
        if (button->m_part == NULL) return -1;
    }
    else {
        button->m_part = NULL;
    }

    return 0;
}

int ui_sprite_spine_ui_anim_button_set_down(ui_sprite_spine_ui_anim_button_t button, const char * down) {
    if (button->m_down) {
        mem_free(button->m_module->m_alloc, button->m_down);
    }

    if (down) {
        button->m_down = cpe_str_mem_dup_trim(button->m_module->m_alloc, down);
        if (button->m_down == NULL) return -1;
    }
    else {
        button->m_down = NULL;
    }

    return 0;
}

int ui_sprite_spine_ui_anim_button_set_up(ui_sprite_spine_ui_anim_button_t button, const char * up) {
    if (button->m_up) {
        mem_free(button->m_module->m_alloc, button->m_up);
    }

    if (up) {
        button->m_up = cpe_str_mem_dup_trim(button->m_module->m_alloc, up);
        if (button->m_up == NULL) return -1;
    }
    else {
        button->m_up = NULL;
    }

    return 0;
}

int ui_sprite_spine_ui_anim_button_init(plugin_ui_animation_t animation, void * ctx) {
    ui_sprite_spine_ui_module_t module = ctx;
    ui_sprite_spine_ui_anim_button_t anim_button = plugin_ui_animation_data(animation);

    plugin_ui_animation_set_auto_free(animation, 0);
    
    anim_button->m_module = module;
    
    anim_button->m_aspect = plugin_ui_aspect_create(plugin_ui_animation_env(animation), NULL);
    if (anim_button->m_aspect == NULL) {
        CPE_ERROR(module->m_em, "ui_sprite_spine_ui_anim_button_init: create asspect fail!");
        return -1;
    }

    anim_button->m_obj = NULL;
    anim_button->m_part = NULL;
    anim_button->m_up = NULL;
    anim_button->m_down = NULL;
    
    return 0;
}

void ui_sprite_spine_ui_anim_button_free(plugin_ui_animation_t animation, void * ctx) {
    ui_sprite_spine_ui_anim_button_t anim_button = plugin_ui_animation_data(animation);

    assert(anim_button->m_aspect);

    plugin_ui_aspect_free(anim_button->m_aspect);

    anim_button->m_aspect = NULL;

    if (anim_button->m_obj) {
        mem_free(anim_button->m_module->m_alloc, anim_button->m_obj);
        anim_button->m_obj = NULL;
    }

    if (anim_button->m_part) {
        mem_free(anim_button->m_module->m_alloc, anim_button->m_part);
        anim_button->m_part = NULL;
    }

    if (anim_button->m_down) {
        mem_free(anim_button->m_module->m_alloc, anim_button->m_down);
        anim_button->m_down = NULL;
    }

    if (anim_button->m_up) {
        mem_free(anim_button->m_module->m_alloc, anim_button->m_up);
        anim_button->m_up = NULL;
    }
}

int ui_sprite_spine_ui_anim_button_add_frame(ui_sprite_spine_ui_anim_button_t anim_button, plugin_ui_control_frame_t frame) {
    plugin_ui_animation_t animation = plugin_ui_animation_from_data(anim_button);
    plugin_ui_control_t control = plugin_ui_control_frame_control(frame);
    plugin_ui_animation_control_t anim_control;
    plugin_ui_control_frame_usage_t usage;
    
    if (plugin_ui_aspect_control_frame_add(anim_button->m_aspect, frame, 0) != 0) {
        CPE_ERROR(anim_button->m_module->m_em, "ui_sprite_spine_ui_anim_button_add_frame: add to aspect fail!");
        return -1;
    }

    anim_control = plugin_ui_animation_control_create(animation, plugin_ui_control_frame_control(frame), 1);
    if (anim_control == NULL) {
        plugin_ui_aspect_control_frame_remove(anim_button->m_aspect, frame);
        CPE_ERROR(anim_button->m_module->m_em, "ui_sprite_spine_ui_anim_button_add_frame: add control to animation fail!");
        return -1;
    }

    usage = plugin_ui_control_frame_usage(frame);
    if (usage == plugin_ui_control_frame_usage_down || usage == plugin_ui_control_frame_usage_normal) {
        plugin_ui_control_frame_t pair_frame;
        
        pair_frame =
            plugin_ui_control_frame_create_by_frame(
                control,
                plugin_ui_control_frame_layer_back,
                usage == plugin_ui_control_frame_usage_down ? plugin_ui_control_frame_usage_normal : plugin_ui_control_frame_usage_down,
                frame);
        if (pair_frame == NULL) {
            CPE_ERROR(anim_button->m_module->m_em, "ui_sprite_spine_ui_anim_button_add_frame: create pair frame fail!");
            plugin_ui_animation_control_free(anim_control);
            plugin_ui_aspect_control_frame_remove(anim_button->m_aspect, frame);
            return -1;
        }

        if (plugin_ui_aspect_control_frame_add(anim_button->m_aspect, pair_frame, 1) != 0) {
            CPE_ERROR(anim_button->m_module->m_em, "ui_sprite_spine_ui_anim_button_add_frame: add to pair frame aspect fail!");
            plugin_ui_control_frame_free(pair_frame);
            plugin_ui_animation_control_free(anim_control);
            plugin_ui_aspect_control_frame_remove(anim_button->m_aspect, frame);
            return -1;
        }
    }

    return 0;
}

int ui_sprite_spine_ui_anim_button_add_control(ui_sprite_spine_ui_anim_button_t anim_button, plugin_ui_control_t control) {
    plugin_ui_animation_t animation = plugin_ui_animation_from_data(anim_button);
    plugin_ui_animation_control_t anim_control;
    
    if (plugin_ui_aspect_control_add(anim_button->m_aspect, control, 0) != 0) {
        CPE_ERROR(anim_button->m_module->m_em, "ui_sprite_spine_ui_anim_button_add_control: add to aspect fail!");
        return -1;
    }

    anim_control = plugin_ui_animation_control_create(animation, control, 1);
    if (anim_control == NULL) {
        plugin_ui_aspect_control_remove(anim_button->m_aspect, control);
        CPE_ERROR(anim_button->m_module->m_em, "ui_sprite_spine_ui_anim_button_add_control: add control to animation fail!");
        return -1;
    }

    return 0;
}

static void ui_sprite_spine_ui_anim_button_apply_transition_render_obj(
    ui_sprite_spine_ui_module_t module, ui_runtime_render_obj_t render_obj, const char * part, const char * transition)
{
    plugin_spine_obj_t spine_obj;
    plugin_spine_obj_part_t obj_part;

    if (ui_runtime_render_obj_type_id(render_obj) != UI_OBJECT_TYPE_SKELETON) {
        CPE_ERROR(
            module->m_em, "ui_sprite_spine_ui_anim_button: %s[%s] is not spine obj",
            ui_runtime_render_obj_name(render_obj),
            ui_data_src_path_dump(&module->m_dump_buffer, ui_runtime_render_obj_src(render_obj)));
        return;
    }
        
    spine_obj = ui_runtime_render_obj_data(render_obj);

    obj_part = plugin_spine_obj_part_find(spine_obj, part);
    if (obj_part == NULL) {
        CPE_ERROR(
            module->m_em, "ui_sprite_spine_ui_anim_button: %s[%s] no part %s",
            ui_runtime_render_obj_name(render_obj),
            ui_data_src_path_dump(&module->m_dump_buffer, ui_runtime_render_obj_src(render_obj)),
            part);
        return;
    }
        
    if (plugin_spine_obj_part_apply_transition_by_name(obj_part, transition) != 0) {
        CPE_ERROR(
            module->m_em, "ui_sprite_spine_ui_anim_button: %s[%s] part %s apply transition %s fail",
            ui_runtime_render_obj_name(render_obj),
            ui_data_src_path_dump(&module->m_dump_buffer, ui_runtime_render_obj_src(render_obj)),
            part,
            transition);
        return;
    }
}

static void ui_sprite_spine_ui_anim_button_apply_transition(
    ui_sprite_spine_ui_anim_button_t anim_button, plugin_ui_control_t from_control, const char * transition)
{
    ui_sprite_spine_ui_module_t module = anim_button->m_module;
    struct plugin_ui_control_frame_it frame_it;
    plugin_ui_control_frame_t frame;
    const char * part = anim_button->m_part ? anim_button->m_part : "main";
    
    assert(transition);

    if (transition[0] == 0) return;
    
    plugin_ui_aspect_control_frames(&frame_it, anim_button->m_aspect);
    while((frame = plugin_ui_control_frame_it_next(&frame_it))) {
        plugin_ui_control_t control = plugin_ui_control_frame_control(frame);
        ui_runtime_render_obj_ref_t render_obj_ref;
        ui_runtime_render_obj_t render_obj;

        if (control != from_control) continue;

        /*frame成对出现，只针对down frame处理 */
        if (plugin_ui_control_frame_usage(frame) != plugin_ui_control_frame_usage_down) continue;
        
        render_obj_ref = plugin_ui_control_frame_render_obj_ref(frame);
        render_obj = ui_runtime_render_obj_ref_obj(render_obj_ref);

        ui_sprite_spine_ui_anim_button_apply_transition_render_obj(module, render_obj, part, transition);
    }

    if (anim_button->m_obj) {
        ui_runtime_render_obj_t render_obj;

        render_obj = ui_runtime_render_obj_find(ui_sprite_ui_module_runtime(module->m_ui_module), anim_button->m_obj);
        if (render_obj == NULL) {
            CPE_ERROR(
                module->m_em, "ui_sprite_spine_ui_anim_button: obj %s not exist",
                anim_button->m_obj);
            return;
        }

        if (ui_runtime_render_obj_type_id(render_obj) != UI_OBJECT_TYPE_SKELETON) {
            CPE_ERROR(
                module->m_em, "ui_sprite_spine_ui_anim_button: obj %s is not spine",
                anim_button->m_obj);
            return;
        }
        
        ui_sprite_spine_ui_anim_button_apply_transition_render_obj(module, render_obj, part, transition);
    }
}

static void ui_sprite_spine_ui_anim_button_on_down(void * ctx, plugin_ui_control_t from_control, plugin_ui_event_t event) {
    ui_sprite_spine_ui_anim_button_t anim_button = ctx;
    ui_sprite_spine_ui_anim_button_apply_transition(
        anim_button, from_control, anim_button->m_down ? anim_button->m_down : "down");
}

static void ui_sprite_spine_ui_anim_button_on_up(void * ctx, plugin_ui_control_t from_control, plugin_ui_event_t event) {
    ui_sprite_spine_ui_anim_button_t anim_button = ctx;
    ui_sprite_spine_ui_anim_button_apply_transition(
        anim_button, from_control, anim_button->m_up ? anim_button->m_up : "up");
}

static int ui_sprite_spine_ui_anim_button_bind_control(
    ui_sprite_spine_ui_module_t module, ui_sprite_spine_ui_anim_button_t anim_button, plugin_ui_control_t control)
{
    plugin_ui_control_action_t action;
    
    action =
        plugin_ui_control_action_create(
            control, plugin_ui_event_mouse_down, plugin_ui_event_scope_self,
            ui_sprite_spine_ui_anim_button_on_down, anim_button);
    if (action == NULL) {
        CPE_ERROR(module->m_em, "ui_sprite_spine_ui_anim_button_enter: create on down action fail");
        return -1;
    }

    if (plugin_ui_aspect_control_action_add(anim_button->m_aspect, action, 1) != 0) {
        CPE_ERROR(module->m_em, "ui_sprite_spine_ui_anim_button_enter: add down action to aspect fail");
        return -1;
    }

    action =
        plugin_ui_control_action_create(
            control, plugin_ui_event_mouse_up, plugin_ui_event_scope_self,
            ui_sprite_spine_ui_anim_button_on_up, anim_button);
    if (action == NULL) {
        CPE_ERROR(module->m_em, "ui_sprite_spine_ui_anim_button_enter: create on up action fail");
        return -1;
    }

    if (plugin_ui_aspect_control_action_add(anim_button->m_aspect, action, 1) != 0) {
        CPE_ERROR(module->m_em, "ui_sprite_spine_ui_anim_button_enter: add up action to aspect fail");
        return -1;
    }

    return 0;
}

int ui_sprite_spine_ui_anim_button_enter(plugin_ui_animation_t animation, void * ctx) {
    ui_sprite_spine_ui_module_t module = ctx;
    ui_sprite_spine_ui_anim_button_t anim_button = plugin_ui_animation_data(animation);
    plugin_ui_control_t control;
    struct plugin_ui_control_it control_it;
    
    plugin_ui_animation_controls(animation, &control_it);
    while((control = plugin_ui_control_it_next(&control_it))) {
        if (ui_sprite_spine_ui_anim_button_bind_control(module, anim_button, control) != 0) goto ENTER_FAIL;
    }
    
    return 0;

ENTER_FAIL:
     plugin_ui_aspect_control_action_clear(anim_button->m_aspect);
     return -1;
}

void ui_sprite_spine_ui_anim_button_exit(plugin_ui_animation_t animation, void * ctx) {
}

int ui_sprite_spine_ui_anim_button_setup(
    plugin_ui_animation_t animation, void * ctx, char * arg_buf_will_change,
    plugin_ui_control_t control, plugin_ui_control_frame_t frame)
{
    ui_sprite_spine_ui_anim_button_t anim_button = plugin_ui_animation_data(animation);
    const char * str_value;

    if ((str_value = cpe_str_read_and_remove_arg(arg_buf_will_change, "spine-ui-button.part", ',', '='))) {
        if (ui_sprite_spine_ui_anim_button_set_part(anim_button, str_value) != 0) {
            CPE_ERROR(anim_button->m_module->m_em, "ui_sprite_spine_ui_anim_button_setup: set part to %s fail", str_value);
            return -1;
        }
    }

    if ((str_value = cpe_str_read_and_remove_arg(arg_buf_will_change, "spine-ui-button.down", ',', '='))) {
        if (ui_sprite_spine_ui_anim_button_set_down(anim_button, str_value) != 0) {
            CPE_ERROR(anim_button->m_module->m_em, "ui_sprite_spine_ui_anim_button_setup: set down to %s fail", str_value);
            return -1;
        }
    }

    if ((str_value = cpe_str_read_and_remove_arg(arg_buf_will_change, "spine-ui-button.up", ',', '='))) {
        if (ui_sprite_spine_ui_anim_button_set_up(anim_button, str_value) != 0) {
            CPE_ERROR(anim_button->m_module->m_em, "ui_sprite_spine_ui_anim_button_setup: set up to %s fail", str_value);
            return -1;
        }
    }

    if (frame) {
        return ui_sprite_spine_ui_anim_button_add_frame(anim_button, frame);
    }
    else if (control) {
        if ((str_value = cpe_str_read_and_remove_arg(arg_buf_will_change, "spine-ui-button.obj", ',', '='))) {
            if (ui_sprite_spine_ui_anim_button_set_obj(anim_button, str_value) != 0) {
                CPE_ERROR(anim_button->m_module->m_em, "ui_sprite_spine_ui_anim_button_setup: set obj to %s fail", str_value);
                return -1;
            }
        }
        else {
            CPE_ERROR(anim_button->m_module->m_em, "ui_sprite_spine_ui_anim_button_setup: bind to control, but obj not configured");
            return -1;
        }
        
        return ui_sprite_spine_ui_anim_button_add_control(anim_button, control);
    }
    else {
        CPE_ERROR(anim_button->m_module->m_em, "ui_sprite_spine_ui_anim_button_setup: no control or frame associate");
        return -1;
    }
}

int ui_sprite_spine_ui_anim_button_regist(ui_sprite_spine_ui_module_t module) {
    plugin_ui_animation_meta_t meta =
        plugin_ui_animation_meta_create(
            ui_sprite_ui_module_ui_module(module->m_ui_module),
            UI_SPRITE_SPINE_UI_ANIM_BUTTON_NAME, module,
            /*animation*/
            sizeof(struct ui_sprite_spine_ui_anim_button),
            ui_sprite_spine_ui_anim_button_init,
            ui_sprite_spine_ui_anim_button_free,
            ui_sprite_spine_ui_anim_button_enter,
            ui_sprite_spine_ui_anim_button_exit,
            NULL,
            /*control*/
            0, NULL, NULL,
            ui_sprite_spine_ui_anim_button_setup);
    
    return meta ? 0 : -1;
}

void ui_sprite_spine_ui_anim_button_unregist(ui_sprite_spine_ui_module_t module) {
    plugin_ui_animation_meta_t meta =
        plugin_ui_animation_meta_find(
            ui_sprite_ui_module_ui_module(module->m_ui_module),
            UI_SPRITE_SPINE_UI_ANIM_BUTTON_NAME);
    if (meta) {
        plugin_ui_animation_meta_free(meta);
    }
}

const char * UI_SPRITE_SPINE_UI_ANIM_BUTTON_NAME = "spine-ui-button";
