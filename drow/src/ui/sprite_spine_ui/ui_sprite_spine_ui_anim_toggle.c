#include <assert.h>
#include "cpe/pal/pal_stdlib.h"
#include "cpe/utils/string_utils.h"
#include "cpe/dr/dr_data_value.h"
#include "gd/app/app_context.h"
#include "render/model/ui_data_src.h"
#include "render/runtime/ui_runtime_render_obj.h"
#include "render/runtime/ui_runtime_render_obj_ref.h"
#include "plugin/spine/plugin_spine_utils.h"
#include "plugin/spine/plugin_spine_obj_part.h"
#include "plugin/spine/plugin_spine_obj_part_state.h"
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
#include "ui_sprite_spine_ui_anim_toggle_i.h"

ui_sprite_spine_ui_anim_toggle_t
ui_sprite_spine_ui_anim_toggle_basic_create(plugin_ui_env_t env) {
    plugin_ui_animation_t animation;

    animation = plugin_ui_animation_create_by_type_name(env, UI_SPRITE_SPINE_UI_ANIM_TOGGLE_NAME);
    if (animation == NULL) return NULL;
    
    return plugin_ui_animation_data(animation);
}

int ui_sprite_spine_ui_anim_toggle_set_obj(ui_sprite_spine_ui_anim_toggle_t toggle, const char * obj) {
    if (toggle->m_obj) {
        mem_free(toggle->m_module->m_alloc, toggle->m_obj);
    }

    if (obj) {
        toggle->m_obj = cpe_str_mem_dup_trim(toggle->m_module->m_alloc, obj);
        if (toggle->m_obj == NULL) return -1;
    }
    else {
        toggle->m_obj = NULL;
    }

    return 0;
}

int ui_sprite_spine_ui_anim_toggle_set_part(ui_sprite_spine_ui_anim_toggle_t toggle, const char * part) {
    if (toggle->m_part) {
        mem_free(toggle->m_module->m_alloc, toggle->m_part);
    }

    if (part) {
        toggle->m_part = cpe_str_mem_dup_trim(toggle->m_module->m_alloc, part);
        if (toggle->m_part == NULL) return -1;
    }
    else {
        toggle->m_part = NULL;
    }

    return 0;
}

int ui_sprite_spine_ui_anim_toggle_set_on(ui_sprite_spine_ui_anim_toggle_t toggle, const char * on) {
    if (toggle->m_on) {
        mem_free(toggle->m_module->m_alloc, toggle->m_on);
    }

    if (on) {
        toggle->m_on = cpe_str_mem_dup_trim(toggle->m_module->m_alloc, on);
        if (toggle->m_on == NULL) return -1;
    }
    else {
        toggle->m_on = NULL;
    }

    return 0;
}

int ui_sprite_spine_ui_anim_toggle_set_off(ui_sprite_spine_ui_anim_toggle_t toggle, const char * off) {
    if (toggle->m_off) {
        mem_free(toggle->m_module->m_alloc, toggle->m_off);
    }

    if (off) {
        toggle->m_off = cpe_str_mem_dup_trim(toggle->m_module->m_alloc, off);
        if (toggle->m_off == NULL) return -1;
    }
    else {
        toggle->m_off = NULL;
    }

    return 0;
}

int ui_sprite_spine_ui_anim_toggle_init(plugin_ui_animation_t animation, void * ctx) {
    ui_sprite_spine_ui_module_t module = ctx;
    ui_sprite_spine_ui_anim_toggle_t anim_toggle = plugin_ui_animation_data(animation);

    plugin_ui_animation_set_auto_free(animation, 0);
    
    anim_toggle->m_module = module;
    
    anim_toggle->m_aspect = plugin_ui_aspect_create(plugin_ui_animation_env(animation), NULL);
    if (anim_toggle->m_aspect == NULL) {
        CPE_ERROR(module->m_em, "ui_sprite_spine_ui_anim_toggle_init: create asspect fail!");
        return -1;
    }

    anim_toggle->m_obj = NULL;
    anim_toggle->m_part = NULL;
    anim_toggle->m_off = NULL;
    anim_toggle->m_on = NULL;
    anim_toggle->m_need_process = 0;

    return 0;
}

void ui_sprite_spine_ui_anim_toggle_free(plugin_ui_animation_t animation, void * ctx) {
    ui_sprite_spine_ui_anim_toggle_t anim_toggle = plugin_ui_animation_data(animation);

    assert(anim_toggle->m_aspect);

    plugin_ui_aspect_free(anim_toggle->m_aspect);

    anim_toggle->m_aspect = NULL;

    if (anim_toggle->m_obj) {
        mem_free(anim_toggle->m_module->m_alloc, anim_toggle->m_obj);
        anim_toggle->m_obj = NULL;
    }

    if (anim_toggle->m_part) {
        mem_free(anim_toggle->m_module->m_alloc, anim_toggle->m_part);
        anim_toggle->m_part = NULL;
    }

    if (anim_toggle->m_on) {
        mem_free(anim_toggle->m_module->m_alloc, anim_toggle->m_on);
        anim_toggle->m_on = NULL;
    }

    if (anim_toggle->m_off) {
        mem_free(anim_toggle->m_module->m_alloc, anim_toggle->m_off);
        anim_toggle->m_off = NULL;
    }
}

static int ui_sprite_spine_ui_anim_toggle_add_control(ui_sprite_spine_ui_anim_toggle_t anim_toggle, plugin_ui_control_t control) {
    plugin_ui_animation_t animation = plugin_ui_animation_from_data(anim_toggle);
    plugin_ui_animation_control_t anim_control;

    if (plugin_ui_control_type(control) != ui_control_type_toggle) {
        CPE_ERROR(
            anim_toggle->m_module->m_em, "ui_sprite_spine_ui_anim_toggle_add_control: control %s is not toggle!",
            plugin_ui_control_path_dump(gd_app_tmp_buffer(anim_toggle->m_module->m_app), control));
        return -1;
    }
    
    if (plugin_ui_aspect_control_add(anim_toggle->m_aspect, control, 0) != 0) {
        CPE_ERROR(anim_toggle->m_module->m_em, "ui_sprite_spine_ui_anim_toggle_add_control: add to aspect fail!");
        return -1;
    }

    anim_control = plugin_ui_animation_control_create(animation, control, 1);
    if (anim_control == NULL) {
        plugin_ui_aspect_control_remove(anim_toggle->m_aspect, control);
        CPE_ERROR(anim_toggle->m_module->m_em, "ui_sprite_spine_ui_anim_toggle_add_control: add control to animation fail!");
        return -1;
    }

    return 0;
}

static void ui_sprite_spine_ui_anim_toggle_on_changed(void * ctx, plugin_ui_control_t from_control, plugin_ui_event_t event) {
    ui_sprite_spine_ui_anim_toggle_t anim_toggle = ctx;
    anim_toggle->m_need_process = 1;
}

static int ui_sprite_spine_ui_anim_toggle_enter(plugin_ui_animation_t animation, void * ctx) {
    ui_sprite_spine_ui_module_t module = ctx;
    ui_sprite_spine_ui_anim_toggle_t anim_toggle = plugin_ui_animation_data(animation);
    plugin_ui_control_t control;
    plugin_ui_control_action_t action;

    control = plugin_ui_animation_find_first_tie_control(animation);
    if (control == NULL) {
        CPE_ERROR(module->m_em, "ui_sprite_spine_ui_anim_toggle_enter: no tie control");
        return -1;
    }
    
    action =
        plugin_ui_control_action_create(
            control, plugin_ui_event_toggle_click, plugin_ui_event_scope_self,
            ui_sprite_spine_ui_anim_toggle_on_changed, anim_toggle);
    if (action == NULL) {
        CPE_ERROR(module->m_em, "ui_sprite_spine_ui_anim_toggle_enter: create action fail");
        return -1;
    }

    if (plugin_ui_aspect_control_action_add(anim_toggle->m_aspect, action, 1) != 0) {
        CPE_ERROR(module->m_em, "ui_sprite_spine_ui_anim_toggle_enter: add action to aspect fail");
        plugin_ui_control_action_free(action);
        return -1;
    }
    
    anim_toggle->m_need_process = 1;
    
    return 0;
}

static uint8_t ui_sprite_spine_ui_anim_toggle_update(plugin_ui_animation_t animation, void * ctx, float delta_s) {
    ui_sprite_spine_ui_module_t module = ctx;
    ui_sprite_spine_ui_anim_toggle_t anim_toggle = plugin_ui_animation_data(animation);
    plugin_ui_control_t control;
    ui_runtime_render_obj_t render_obj;
    plugin_spine_obj_t spine_obj;
    const char * part_name;
    plugin_spine_obj_part_t obj_part;
    const char * target_state_name;
    plugin_spine_obj_part_state_t target_state;
    plugin_spine_obj_part_transition_t transition;
    struct dr_value value_buf;
    uint8_t pushed;
    
    if (anim_toggle->m_need_process == 0) return 1;

    control = plugin_ui_animation_find_first_tie_control(animation);
    if (control == NULL) {
        CPE_ERROR(module->m_em, "ui_sprite_spine_ui_anim_toggle_enter: no tie control");
        return 0;
    }

    render_obj = NULL;
    if (anim_toggle->m_obj) {
        render_obj = ui_runtime_render_obj_find(ui_sprite_ui_module_runtime(module->m_ui_module), anim_toggle->m_obj);
        if (render_obj == NULL) {
            CPE_ERROR(
                module->m_em, "ui_sprite_spine_ui_anim_toggle: obj %s not exist",
                anim_toggle->m_obj);
            return 0;
        }
    }
    else {
        struct plugin_ui_control_frame_it frame_it;
        plugin_ui_control_frame_t frame;
        
        plugin_ui_aspect_control_frames(&frame_it, anim_toggle->m_aspect);
        while((frame = plugin_ui_control_frame_it_next(&frame_it))) {
            render_obj = ui_runtime_render_obj_ref_obj(plugin_ui_control_frame_render_obj_ref(frame));
            break;
        }
    }

    if (render_obj == NULL) {
        CPE_ERROR(module->m_em, "ui_sprite_spine_ui_anim_toggle: no render obj");
        return 0;
    }

    if (ui_runtime_render_obj_type_id(render_obj) != UI_OBJECT_TYPE_SKELETON) {
        CPE_ERROR(
            module->m_em, "ui_sprite_spine_ui_anim_toggle: render obj %s[%s] is not spine",
            ui_runtime_render_obj_name(render_obj),
            ui_data_src_path_dump(&module->m_dump_buffer, ui_runtime_render_obj_src(render_obj)));
        return 0;
    }

    spine_obj = ui_runtime_render_obj_data(render_obj);
    
    part_name = anim_toggle->m_part ? anim_toggle->m_part : "main";
    obj_part = plugin_spine_obj_part_find(spine_obj, part_name);
    if (obj_part == NULL) {
        CPE_ERROR(
            module->m_em, "ui_sprite_spine_ui_anim_toggle: %s[%s] no part %s",
            ui_runtime_render_obj_name(render_obj),
            ui_data_src_path_dump(&module->m_dump_buffer, ui_runtime_render_obj_src(render_obj)),
            part_name);
        return 0;
    }

    if (plugin_spine_obj_part_is_in_enter(obj_part)) return 1;

    if (plugin_ui_control_get_attr(control, "pushed", &value_buf) != 0) {
        CPE_ERROR(
            module->m_em, "ui_sprite_spine_ui_anim_toggle: %s[%s] read pushed fail!",
            ui_runtime_render_obj_name(render_obj),
            ui_data_src_path_dump(&module->m_dump_buffer, ui_runtime_render_obj_src(render_obj)));
        return 0;
    }

    pushed = dr_value_read_with_dft_uint8(&value_buf, 0);

    if (pushed) {
        target_state_name = anim_toggle->m_on ? anim_toggle->m_on : "on";
    }
    else {
        target_state_name = anim_toggle->m_off ? anim_toggle->m_off : "off";
    }

    target_state = plugin_spine_obj_part_state_find(obj_part, target_state_name);
    if (target_state == NULL) {
        CPE_ERROR(
            module->m_em, "ui_sprite_spine_ui_anim_toggle: %s[%s] part %s no state %s!",
            ui_runtime_render_obj_name(render_obj),
            ui_data_src_path_dump(&module->m_dump_buffer, ui_runtime_render_obj_src(render_obj)),
            part_name, target_state_name);
        return 0;
    }

    if (plugin_spine_obj_part_cur_state(obj_part) == target_state) {
        anim_toggle->m_need_process = 0;
        return 1;
    }

    transition = plugin_spine_obj_part_transition_find_by_target(plugin_spine_obj_part_cur_state(obj_part), target_state_name);
    if (transition == NULL) {
        CPE_ERROR(
            module->m_em, "ui_sprite_spine_ui_anim_toggle: %s[%s] part %s state %s transition to state %s!",
            ui_runtime_render_obj_name(render_obj),
            ui_data_src_path_dump(&module->m_dump_buffer, ui_runtime_render_obj_src(render_obj)),
            part_name,
            plugin_spine_obj_part_state_name(plugin_spine_obj_part_cur_state(obj_part)),
            target_state_name);
        return 0;
    }

    if (plugin_spine_obj_part_apply_transition(obj_part, transition) != 0) {
        CPE_ERROR(
            module->m_em, "ui_sprite_spine_ui_anim_toggle: %s[%s] part %s apply transition %s fail",
            ui_runtime_render_obj_name(render_obj),
            ui_data_src_path_dump(&module->m_dump_buffer, ui_runtime_render_obj_src(render_obj)),
            part_name,
            plugin_spine_obj_part_transition_name(transition));
        return 0;
    }
    
    anim_toggle->m_need_process = 0;
    return 1;
}

static void ui_sprite_spine_ui_anim_toggle_exit(plugin_ui_animation_t animation, void * ctx) {
}

static int ui_sprite_spine_ui_anim_toggle_setup(
    plugin_ui_animation_t animation, void * ctx, char * arg_buf_will_change,
    plugin_ui_control_t control, plugin_ui_control_frame_t frame)
{
    ui_sprite_spine_ui_anim_toggle_t anim_toggle = plugin_ui_animation_data(animation);
    const char * str_value;

    if ((str_value = cpe_str_read_and_remove_arg(arg_buf_will_change, "spine-ui-toggle.part", ',', '='))) {
        if (ui_sprite_spine_ui_anim_toggle_set_part(anim_toggle, str_value) != 0) {
            CPE_ERROR(anim_toggle->m_module->m_em, "ui_sprite_spine_ui_anim_toggle_setup: set part to %s fail", str_value);
            return -1;
        }
    }

    if ((str_value = cpe_str_read_and_remove_arg(arg_buf_will_change, "spine-ui-toggle.on", ',', '='))) {
        if (ui_sprite_spine_ui_anim_toggle_set_on(anim_toggle, str_value) != 0) {
            CPE_ERROR(anim_toggle->m_module->m_em, "ui_sprite_spine_ui_anim_toggle_setup: set on to %s fail", str_value);
            return -1;
        }
    }

    if ((str_value = cpe_str_read_and_remove_arg(arg_buf_will_change, "spine-ui-toggle.off", ',', '='))) {
        if (ui_sprite_spine_ui_anim_toggle_set_off(anim_toggle, str_value) != 0) {
            CPE_ERROR(anim_toggle->m_module->m_em, "ui_sprite_spine_ui_anim_toggle_setup: set off to %s fail", str_value);
            return -1;
        }
    }

    if (control == NULL) {
        CPE_ERROR(anim_toggle->m_module->m_em, "ui_sprite_spine_ui_anim_toggle_setup: no control or frame associate");
        return -1;
    }
    
    if ((str_value = cpe_str_read_and_remove_arg(arg_buf_will_change, "spine-ui-toggle.obj", ',', '='))) {
        if (ui_sprite_spine_ui_anim_toggle_set_obj(anim_toggle, str_value) != 0) {
            CPE_ERROR(anim_toggle->m_module->m_em, "ui_sprite_spine_ui_anim_toggle_setup: set obj to %s fail", str_value);
            return -1;
        }
    }
    else {
        CPE_ERROR(anim_toggle->m_module->m_em, "ui_sprite_spine_ui_anim_toggle_setup: bind to control, but obj not configured");
        return -1;
    }
        
    return ui_sprite_spine_ui_anim_toggle_add_control(anim_toggle, control);
}

int ui_sprite_spine_ui_anim_toggle_regist(ui_sprite_spine_ui_module_t module) {
    plugin_ui_animation_meta_t meta =
        plugin_ui_animation_meta_create(
            ui_sprite_ui_module_ui_module(module->m_ui_module),
            UI_SPRITE_SPINE_UI_ANIM_TOGGLE_NAME, module,
            /*animation*/
            sizeof(struct ui_sprite_spine_ui_anim_toggle),
            ui_sprite_spine_ui_anim_toggle_init,
            ui_sprite_spine_ui_anim_toggle_free,
            ui_sprite_spine_ui_anim_toggle_enter,
            ui_sprite_spine_ui_anim_toggle_exit,
            ui_sprite_spine_ui_anim_toggle_update,
            /*control*/
            0, NULL, NULL,
            ui_sprite_spine_ui_anim_toggle_setup);
    
    return meta ? 0 : -1;
}

void ui_sprite_spine_ui_anim_toggle_unregist(ui_sprite_spine_ui_module_t module) {
    plugin_ui_animation_meta_t meta =
        plugin_ui_animation_meta_find(
            ui_sprite_ui_module_ui_module(module->m_ui_module),
            UI_SPRITE_SPINE_UI_ANIM_TOGGLE_NAME);
    if (meta) {
        plugin_ui_animation_meta_free(meta);
    }
}

const char * UI_SPRITE_SPINE_UI_ANIM_TOGGLE_NAME = "spine-ui-toggle";
