#include <assert.h>
#include "cpe/utils/string_utils.h"
#include "cpe/utils/stream_buffer.h"
#include "render/model/ui_data_layout.h"
#include "render/runtime/ui_runtime_module.h"
#include "render/cache/ui_cache_color.h"
#include "plugin_ui_control_i.h"
#include "plugin_ui_aspect_i.h"
#include "plugin_ui_aspect_ref_i.h"
#include "plugin_ui_page_i.h"
#include "plugin_ui_control_attr_meta_i.h"
#include "plugin_ui_control_action_slots_i.h"
#include "plugin_ui_control_timer_i.h"
#include "plugin_ui_control_binding_i.h"
#include "plugin_ui_control_frame_i.h"
#include "plugin_ui_touch_track_i.h"
#include "plugin_ui_animation_control_i.h"
#include "plugin_ui_animation_i.h"
#include "plugin_ui_control_category_i.h"

plugin_ui_control_t
plugin_ui_control_create(plugin_ui_page_t page, uint8_t type) {
    plugin_ui_env_t env = page->m_env;
    plugin_ui_module_t module = env->m_module;
    plugin_ui_control_t control;
    plugin_ui_control_meta_t meta;

    meta = plugin_ui_control_meta_find(module, type);
    if (meta == NULL) {
        CPE_ERROR(module->m_em, "plugin_ui_control: type %s no control meta!", ui_data_control_type_name(type));
        return NULL;
    }

    control = TAILQ_FIRST(&env->m_free_controls);
    if (control) {
        env->m_control_free_count--;
        TAILQ_REMOVE(&env->m_free_controls, control, m_next_for_parent);
    }
    else {
        control = mem_alloc(module->m_alloc, sizeof(struct plugin_ui_control) + module->m_control_max_capacity);
        if (control == NULL) {
            CPE_ERROR(module->m_em, "plugin_ui_control: create: alloc fail!");
            return NULL;
        }
    }

    if (plugin_ui_control_do_init(page, control, meta) != 0) {
        CPE_ERROR(module->m_em, "plugin_ui_control: create: init fail!");
        control->m_page = (void*)env;
        env->m_control_free_count++;
        TAILQ_INSERT_TAIL(&env->m_free_controls, control, m_next_for_parent);
        return NULL;
    }

    TAILQ_INIT(&control->m_animations);

    env->m_control_count++;
    page->m_control_count++;

    TAILQ_INSERT_TAIL(&meta->m_controls, control, m_next_for_meta);
    TAILQ_INSERT_TAIL(&page->m_building_controls, control, m_next_for_parent);

    return control;
}

void plugin_ui_control_free(plugin_ui_control_t control) {
    plugin_ui_env_t env = control->m_page->m_env;
    plugin_ui_touch_track_t track;

    if (control->m_is_processing) {
        assert(!control->m_is_free);
        control->m_is_free = 1;

        plugin_ui_control_cancel_animations(control, NULL);

        if (control->m_parent) {
            plugin_ui_control_evt_on_control_remove(env, control);
            TAILQ_REMOVE(&control->m_parent->m_childs, control, m_next_for_parent);
            TAILQ_INSERT_TAIL(&control->m_page->m_building_controls, control, m_next_for_parent);
            control->m_parent = NULL;
        }

        if ((track = plugin_ui_control_touch_track(control))) {
            track->m_catch_control = NULL;
        }
        
        while(!TAILQ_EMPTY(&control->m_aspects)) {
            plugin_ui_aspect_ref_t ref = TAILQ_FIRST(&control->m_aspects);
            plugin_ui_aspect_ref_free(ref, &ref->m_aspect->m_controls, &control->m_aspects);
        }

        while(!TAILQ_EMPTY(&control->m_childs)) {
            plugin_ui_control_free(TAILQ_FIRST(&control->m_childs));
        }
        
        return;
    }
    control->m_is_free = 1;

    plugin_ui_control_cancel_animations(control, NULL);
    
    plugin_ui_control_do_fini(control);

    while(!TAILQ_EMPTY(&control->m_childs)) {
        plugin_ui_control_free(TAILQ_FIRST(&control->m_childs));
    }
    
    if (control->m_parent) {
        plugin_ui_control_evt_on_control_remove(env, control);
        TAILQ_REMOVE(&control->m_parent->m_childs, control, m_next_for_parent);
    }
    else {
        TAILQ_REMOVE(&control->m_page->m_building_controls, control, m_next_for_parent);
    }

    while(!TAILQ_EMPTY(&control->m_aspects)) {
        plugin_ui_aspect_ref_t ref = TAILQ_FIRST(&control->m_aspects);
        plugin_ui_aspect_ref_free(ref, &ref->m_aspect->m_controls, &control->m_aspects);
    }

    assert(env->m_control_count > 0);
    env->m_control_count--;
    assert(control->m_page->m_control_count > 0);
    control->m_page->m_control_count--;

    TAILQ_REMOVE(&control->m_meta->m_controls, control, m_next_for_meta);
    
    control->m_page = (void*)env;
    env->m_control_free_count++;
    TAILQ_INSERT_TAIL(&env->m_free_controls, control, m_next_for_parent);
}

ui_runtime_module_t plugin_ui_control_runtime(plugin_ui_control_t control) {
    return control->m_page->m_env->m_module->m_runtime;
}

void plugin_ui_control_real_free(plugin_ui_control_t control) {
    plugin_ui_env_t env = (void*)control->m_page;

    assert(env->m_control_free_count > 0);
    env->m_control_free_count--;
    TAILQ_REMOVE(&env->m_free_controls, control, m_next_for_parent);

    mem_free(env->m_module->m_alloc, control);
}

void * plugin_ui_control_product(plugin_ui_control_t control) {
    return control + 1;
}

uint32_t plugin_ui_control_product_capacity(plugin_ui_control_t control) {
    return control->m_meta->m_product_capacity;
}

plugin_ui_control_t plugin_ui_control_from_product(void * data) {
    return ((plugin_ui_control_t)data) - 1;
}

gd_app_context_t plugin_ui_control_app(plugin_ui_control_t control) {
    return control->m_page->m_env->m_module->m_app;
}

plugin_ui_env_t plugin_ui_control_env(plugin_ui_control_t control) {
    return control->m_page->m_env;
}

plugin_ui_page_t plugin_ui_control_page(plugin_ui_control_t control) {
    return control->m_page;
}

ui_cache_manager_t plugin_ui_control_cache_mgr(plugin_ui_control_t control) {
    return ui_runtime_module_cache_mgr(control->m_page->m_env->m_module->m_runtime);
}

ui_data_mgr_t plugin_ui_control_data_mgr(plugin_ui_control_t control) {
    return control->m_page->m_env->m_module->m_data_mgr;
}

uint8_t plugin_ui_control_type(plugin_ui_control_t control) {
    return control->m_meta->m_type;
}

const char * plugin_ui_control_name(plugin_ui_control_t control) {
    if (control->m_name) return control->m_name;
    if (control->m_src) return ui_data_control_msg(control->m_src, ui_data_control_data(control->m_src)->name_id);
    return "";
}

void plugin_ui_control_set_name(plugin_ui_control_t control, const char * name) {
    plugin_ui_module_t module = control->m_page->m_env->m_module;
    
    if (control->m_name) {
        mem_free(module->m_alloc, control->m_name);
    }

    if (name) {
        control->m_name = cpe_str_mem_dup(module->m_alloc, name);
    }
    else {
        control->m_name = NULL;
    }
}

plugin_ui_aspect_t plugin_ui_control_lock_aspect(plugin_ui_control_t control) {
    return control->m_page->m_env->m_lock_aspect;
}

static void plugin_ui_control_path_print(write_stream_t s, plugin_ui_control_t control) {
    if (control->m_parent) {
        plugin_ui_control_path_print(s, control->m_parent);
        stream_printf(s, ".%s", plugin_ui_control_name(control));        
    }
    else {
        stream_printf(s, "%s", plugin_ui_page_name(control->m_page));
    }
}

const char * plugin_ui_control_path_dump(mem_buffer_t buffer, plugin_ui_control_t control) {
    struct write_stream_buffer stream = CPE_WRITE_STREAM_BUFFER_INITIALIZER(buffer);

    mem_buffer_clear_data(buffer);

    plugin_ui_control_path_print((write_stream_t)&stream, control);

    stream_putc((write_stream_t)&stream, 0);

    return (const char *)mem_buffer_make_continuous(buffer, 0);
}

const char * plugin_ui_control_user_text(plugin_ui_control_t control) {
    if (control->m_user_text) return control->m_user_text;
    if (control->m_src) return plugin_ui_control_msg(control, ui_data_control_data(control->m_src)->basic.user_text_id);
    return "";
}

void plugin_ui_control_set_user_text(plugin_ui_control_t control, const char * user_text) {
    plugin_ui_module_t module = control->m_page->m_env->m_module;
    
    if (control->m_user_text) {
        mem_free(module->m_alloc, control->m_user_text);
    }

    control->m_user_text = cpe_str_mem_dup(module->m_alloc, user_text);
}

ui_data_control_anim_t
plugin_ui_control_find_anim_data(plugin_ui_control_t control, uint8_t anim_type) {
    return control->m_src
        ? ui_data_control_anim_find(control->m_src, anim_type)
        : NULL;     
}

uint8_t plugin_ui_control_has_focus(plugin_ui_control_t control) {
    return control->m_page->m_env->m_focus_control == control ? 1 : 0;
}

void plugin_ui_control_set_focus(plugin_ui_control_t control) {
    plugin_ui_env_set_focus_control(control->m_page->m_env, control);
}

int plugin_ui_control_do_init(plugin_ui_page_t page, plugin_ui_control_t control, plugin_ui_control_meta_t meta) {
    control->m_meta = meta;
    control->m_page = page;
    control->m_parent = NULL;
    control->m_child_count = 0;
    control->m_action_slots = NULL;
    TAILQ_INIT(&control->m_aspects);
    TAILQ_INIT(&control->m_timers);
    TAILQ_INIT(&control->m_bindings);
    TAILQ_INIT(&control->m_frames);
    control->m_is_processing = 0;
    control->m_is_free = 0;

    /*初始化数据 */
    control->m_src = NULL;
    control->m_template = NULL;
    control->m_name = NULL;
    control->m_user_text = NULL;

    TAILQ_INIT(&control->m_childs);

    plugin_ui_control_do_clear(control);
    
    if (meta->m_init) {
        if (meta->m_init(control) != 0) return -1;
    }
    
    return 0;
}

ui_data_control_t plugin_ui_control_src(plugin_ui_control_t control) {
    return control->m_src;
}

ui_data_control_t plugin_ui_control_data_src(plugin_ui_control_t control) {
    return control->m_template ? control->m_template : control->m_src;
}

void plugin_ui_control_do_clear(plugin_ui_control_t control) {
    plugin_ui_module_t module = control->m_page->m_env->m_module;
    uint8_t i;
    
    control->m_src = NULL;
    control->m_template = NULL;

    control->m_flag = 0;
    cpe_ba_set(&control->m_flag, plugin_ui_control_flag_visible, cpe_ba_true);
    cpe_ba_set(&control->m_flag, plugin_ui_control_flag_enable, cpe_ba_true);
    cpe_ba_set(&control->m_flag, plugin_ui_control_flag_accept_global_disable_color, cpe_ba_true);
    
    control->m_render_usables = 0;
    cpe_ba_set(&control->m_render_usables, plugin_ui_control_frame_usage_normal, cpe_ba_true);
    control->m_render_layers = 0;
    cpe_ba_set(&control->m_render_layers, plugin_ui_control_frame_layer_back, cpe_ba_true);
    cpe_ba_set(&control->m_render_layers, plugin_ui_control_frame_layer_text, cpe_ba_true);
    cpe_ba_set(&control->m_render_layers, plugin_ui_control_frame_layer_tail, cpe_ba_true);
    
    control->m_cache_flag = 0;
    for(i = 0; i < plugin_ui_control_cache_flag_count; ++i) {
        cpe_ba_set(&control->m_cache_flag, i, cpe_ba_true);
    }

	control->m_align_horz = ui_align_mode_horz_none;
	control->m_align_vert = ui_align_mode_vert_none;

    control->m_all_frame_pt = UI_VECTOR_2_ZERO;
    control->m_all_frame_scale = UI_VECTOR_2_IDENTITY;
    
    control->m_editor_pt = UI_VECTOR_2_ZERO;
    control->m_editor_sz = control->m_page->m_env->m_runtime_sz;
    control->m_editor_pd.lt = control->m_editor_pt;
    control->m_editor_pd.rb = control->m_editor_sz;

    control->m_render_pt.x.k = 0.0f;
    control->m_render_pt.x.b = 0.0f;
    control->m_render_pt.y.k = 0.0f;
    control->m_render_pt.y.b = 0.0f;

    control->m_render_sz.x.k = 1.0f;
    control->m_render_sz.x.b = 0.0f;
    control->m_render_sz.y.k = 1.0f;
    control->m_render_sz.y.b = 0.0f;

    control->m_client_sz.x.k = 1.0f;
    control->m_client_sz.x.b = 0.0f;
    control->m_client_sz.y.k = 1.0f;
    control->m_client_sz.y.b = 0.0f;

    control->m_client_pd.lt.x.k = 0.0f;
    control->m_client_pd.lt.x.b = 0.0f;
    control->m_client_pd.lt.y.k = 0.0f;
    control->m_client_pd.lt.y.b = 0.0f;

    control->m_client_pd.rb.x.k = 1.0f;
    control->m_client_pd.rb.x.b = 0.0f;
    control->m_client_pd.rb.y.k = 1.0f;
    control->m_client_pd.rb.y.b = 0.0f;
    
    control->m_scroll_pt.x = 0.0f;
    control->m_scroll_pt.y = 0.0f;

    control->m_pivot = UI_VECTOR_2_ZERO;
    control->m_angle = UI_VECTOR_3_ZERO;
    control->m_scale = UI_VECTOR_2_IDENTITY;
    control->m_float_scale = UI_VECTOR_2_IDENTITY;

    control->m_alpha = 1.0f;
    control->m_color = UI_COLOR_WHITE;
    ui_cache_find_color(ui_runtime_module_cache_mgr(module->m_runtime), "gray", &control->m_gray_color);

    control->m_draw_align = 0;
    
    if (control->m_name) {
        mem_free(module->m_alloc, control->m_name);
        control->m_name = NULL;
    }

    if (control->m_user_text) {
        mem_free(module->m_alloc, control->m_user_text);
        control->m_user_text = NULL;
    }
}

void plugin_ui_control_do_fini(plugin_ui_control_t control) {
    while(!TAILQ_EMPTY(&control->m_childs)) {
        plugin_ui_control_free(TAILQ_FIRST(&control->m_childs));
    }

    if (control->m_meta->m_fini) control->m_meta->m_fini(control);

    plugin_ui_control_do_clear(control);

    if (control->m_action_slots) {
        plugin_ui_control_action_slots_free(control, control->m_action_slots);
        control->m_action_slots = NULL;
    }

    while(!TAILQ_EMPTY(&control->m_timers)) {
        plugin_ui_control_timer_free(TAILQ_FIRST(&control->m_timers));
    }

    while(!TAILQ_EMPTY(&control->m_bindings)) {
        plugin_ui_control_binding_free(TAILQ_FIRST(&control->m_bindings));
    }
    
    while(!TAILQ_EMPTY(&control->m_frames)) {
        plugin_ui_control_frame_free(TAILQ_FIRST(&control->m_frames));
    }
}

mem_allocrator_t plugin_ui_control_allocrator(plugin_ui_control_t control) {
    return control->m_page->m_env->m_module->m_alloc;
}

int plugin_ui_control_set_attr(plugin_ui_control_t control, const char * attr_name, dr_value_t attr_value) {
    plugin_ui_env_t env = control->m_page->m_env;
    plugin_ui_control_attr_meta_t attr_meta;

    attr_meta = plugin_ui_control_attr_meta_find(control->m_meta, attr_name);
    if (attr_meta == NULL) {
        CPE_ERROR(
            env->m_module->m_em, "plugin_ui_control_set_attr: control type %s attr %s not exist!",
            ui_data_control_type_name(control->m_meta->m_type), attr_name);
        return -1;
    }

    if (attr_meta->m_setter == NULL) {
        CPE_ERROR(
            env->m_module->m_em, "plugin_ui_control_set_attr: control type %s attr %s not support set!",
            ui_data_control_type_name(control->m_meta->m_type), attr_name);
        return -1;
    }

    if (attr_meta->m_setter(control, attr_value) != 0) {
        CPE_ERROR(
            env->m_module->m_em, "plugin_ui_control_set_attr: control type %s attr %s set value fail!",
            ui_data_control_type_name(control->m_meta->m_type), attr_name);
        return -1;
    }

    return 0;
}

int plugin_ui_control_set_attr_by_str(plugin_ui_control_t control, const char * attr_name, const char * attr_value) {
    plugin_ui_env_t env = control->m_page->m_env;
    plugin_ui_control_attr_meta_t attr_meta;
    struct dr_value v;

    attr_meta = plugin_ui_control_attr_meta_find(control->m_meta, attr_name);
    if (attr_meta == NULL) {
        CPE_ERROR(
            env->m_module->m_em, "plugin_ui_control_set_attr: control type %s attr %s not exist!",
            ui_data_control_type_name(control->m_meta->m_type), attr_name);
        return -1;
    }

    if (attr_meta->m_setter == NULL) {
        CPE_ERROR(
            env->m_module->m_em, "plugin_ui_control_set_attr: control type %s attr %s not support set!",
            ui_data_control_type_name(control->m_meta->m_type), attr_name);
        return -1;
    }

    v.m_type = CPE_DR_TYPE_STRING;
    v.m_meta = NULL;
    v.m_data = (void*)attr_value;
    v.m_size = strlen(attr_value) + 1;
    
    if (attr_meta->m_setter(control, &v) != 0) {
        CPE_ERROR(
            env->m_module->m_em, "plugin_ui_control_set_attr: control type %s attr %s set value from str %s fail!",
            ui_data_control_type_name(control->m_meta->m_type), attr_name, attr_value);
        return -1;
    }
    
    return 0;
}

int plugin_ui_control_get_attr(plugin_ui_control_t control, const char * attr_name, dr_value_t attr_value) {
    plugin_ui_env_t env = control->m_page->m_env;
    plugin_ui_control_attr_meta_t attr_meta;

    attr_meta = plugin_ui_control_attr_meta_find(control->m_meta, attr_name);
    if (attr_meta == NULL) {
        CPE_ERROR(
            env->m_module->m_em, "plugin_ui_control_get_attr: control type %s attr %s not exist!",
            ui_data_control_type_name(control->m_meta->m_type), attr_name);
        return -1;
    }

    if (attr_meta->m_getter == NULL) {
        CPE_ERROR(
            env->m_module->m_em, "plugin_ui_control_get_attr: control type %s attr %s not support get!",
            ui_data_control_type_name(control->m_meta->m_type), attr_name);
        return -1;
    }

    if (attr_meta->m_getter(control, attr_value) != 0) {
        CPE_ERROR(
            env->m_module->m_em, "plugin_ui_control_get_attr: control type %s attr %s get value fail!",
            ui_data_control_type_name(control->m_meta->m_type), attr_name);
        return -1;
    }

    return 0;
}

struct plugin_ui_control_bulk_set_attrs_ctx {
    plugin_ui_control_t m_control;
    int m_rv;
};

static void plugin_ui_control_bulk_set_attrs_do(void * i_ctx, const char * value) {
    struct plugin_ui_control_bulk_set_attrs_ctx * ctx = i_ctx;
    const char * sep;
    char name_buf[64];

    if (value[0] == 0) return;
    
    if ((sep = strchr(value, '=')) == NULL) {
        CPE_ERROR(
            ctx->m_control->m_page->m_env->m_module->m_em,
            "plugin_ui_control_set_attr: setter %s format error", value);
        ctx->m_rv = -1;
        return;
    }

    if (cpe_str_dup_range(name_buf, sizeof(name_buf), value, sep) == NULL) {
        CPE_ERROR(
            ctx->m_control->m_page->m_env->m_module->m_em,
            "plugin_ui_control_set_attr: setter %s attr name overflow", value);
        ctx->m_rv = -1;
        return;
    }

    if (plugin_ui_control_set_attr_by_str(ctx->m_control, name_buf, sep + 1) != 0) ctx->m_rv = -1;
}

int plugin_ui_control_bulk_set_attrs(plugin_ui_control_t control, const char * attrs) {
    struct plugin_ui_control_bulk_set_attrs_ctx ctx;
    ctx.m_control = control;
    ctx.m_rv = 0;
    cpe_str_list_for_each(attrs, ';', plugin_ui_control_bulk_set_attrs_do, &ctx);
    return ctx.m_rv;
}

void plugin_ui_control_cancel_animations(plugin_ui_control_t control, plugin_ui_aspect_t aspect) {
    plugin_ui_animation_control_t animation_control, next;
    
    for(animation_control = TAILQ_FIRST(&control->m_animations); animation_control; animation_control = next) {
        next = TAILQ_NEXT(animation_control, m_next_for_control);

        if (aspect == NULL || plugin_ui_aspect_animation_is_in(aspect, animation_control->m_animation)) {
            if (animation_control->m_is_tie) {
                plugin_ui_animation_free(animation_control->m_animation);
            }
            else {
                plugin_ui_animation_control_free(animation_control);
            }
        }
    }
}

plugin_ui_animation_t
plugin_ui_control_create_animation(plugin_ui_control_t control, char * str_value) {
    plugin_ui_env_t env = control->m_page->m_env;
    const char * anim_type;
    char * anim_args;
    char * sep;
    plugin_ui_animation_meta_t animation_meta;
    plugin_ui_animation_t animation;

    sep = strchr(str_value, ':');
    if (sep == NULL) {
        * cpe_str_trim_tail(str_value + strlen(str_value), str_value) = 0;
        anim_type = str_value;
        anim_args = NULL;
    }
    else {
        * cpe_str_trim_tail(sep, str_value) = 0;
        anim_type = str_value;
        anim_args = cpe_str_trim_head(sep + 1);
    }

    animation_meta = plugin_ui_animation_meta_find(env->m_module, anim_type);
    if (animation_meta == NULL) {
        CPE_ERROR(env->m_module->m_em, "plugin_ui_control_create_animation: animation type %s not exist", anim_type);
        return NULL;
    }

    animation = plugin_ui_animation_create(env, animation_meta);
    if (animation == NULL) {
        CPE_ERROR(env->m_module->m_em, "plugin_ui_control_create_animation: animation type %s create fail!", anim_type);
        return NULL;
    }

    if (anim_args) {
        if (plugin_ui_animation_setup(animation, anim_args, control, NULL) != 0) {
            CPE_ERROR(env->m_module->m_em, "plugin_ui_control_create_animation: animation type %s setup fail!", anim_type);
            plugin_ui_animation_free(animation);
            return NULL;
        }
    }

    return animation;
}

plugin_ui_animation_t
plugin_ui_control_find_animation_by_name(plugin_ui_control_t control, const char * name) {
    plugin_ui_animation_control_t animation_control;

    TAILQ_FOREACH(animation_control, &control->m_animations, m_next_for_control) {
        if (strcmp(animation_control->m_animation->m_meta->m_name, name) == 0) {
            return animation_control->m_animation;
        }
    }
    
    return NULL;
}

plugin_ui_animation_t
plugin_ui_control_find_animation_by_type_name(plugin_ui_control_t control, const char * name) {
    plugin_ui_animation_control_t animation_control;

    TAILQ_FOREACH(animation_control, &control->m_animations, m_next_for_control) {
        if (animation_control->m_animation->m_name && strcmp(animation_control->m_animation->m_name, name) == 0) {
            return animation_control->m_animation;
        }
    }
    
    return NULL;
}

struct plugin_ui_control_animation_it_data {
    plugin_ui_animation_control_t m_anim_control;
    const char * m_type_name;
    plugin_ui_aspect_t m_aspect;
};

static void plugin_ui_control_animation_check_go_next(struct plugin_ui_control_animation_it_data * data) {
    for(; data->m_anim_control; data->m_anim_control = TAILQ_NEXT(data->m_anim_control, m_next_for_control)) {
        if (data->m_type_name) {
            if (strcmp(plugin_ui_animation_type_name(data->m_anim_control->m_animation), data->m_type_name) != 0) continue;
        }

        if (data->m_aspect) {
            if (!plugin_ui_aspect_animation_is_in(data->m_aspect, data->m_anim_control->m_animation)) continue;
        }

        return;
    }
}
    
static plugin_ui_animation_t plugin_ui_control_animation_next(struct plugin_ui_animation_it * it) {
    struct plugin_ui_control_animation_it_data * data = (struct plugin_ui_control_animation_it_data *)(it->m_data);
    plugin_ui_animation_control_t r;
    if (data->m_anim_control == NULL) return NULL;
    r = data->m_anim_control;
    
    data->m_anim_control = TAILQ_NEXT(data->m_anim_control, m_next_for_control);
    plugin_ui_control_animation_check_go_next(data);

    return r->m_animation;
}

void plugin_ui_control_animations(plugin_ui_control_t control, plugin_ui_animation_it_t it, const char * type_name, plugin_ui_aspect_t aspect) {
    struct plugin_ui_control_animation_it_data * data = (struct plugin_ui_control_animation_it_data *)(it->m_data);
    data->m_anim_control = TAILQ_FIRST(&control->m_animations);
    data->m_type_name = type_name;
    data->m_aspect = aspect;
    it->next = plugin_ui_control_animation_next;
    plugin_ui_control_animation_check_go_next(data);
}

int plugin_ui_control_play_event_sfx(plugin_ui_control_t control, plugin_ui_event_t evt) {
    plugin_ui_env_t env = control->m_page->m_env;
    const char * sound_sfx = NULL;

    UI_CONTROL const * control_data = control->m_src ? ui_data_control_data(control->m_src) : NULL;
    if (control_data) {
        switch(evt) {
        case plugin_ui_event_mouse_click:
            if (control_data->basic.push_sfx_file_id) {
                sound_sfx = plugin_ui_control_msg(control, control_data->basic.push_sfx_file_id);
            }
            break;
        case plugin_ui_event_mouse_down:
            if (control_data->basic.down_sfx_file_id) {
                sound_sfx = plugin_ui_control_msg(control, control_data->basic.down_sfx_file_id);
            }
            break;
        case plugin_ui_event_mouse_up:
            if (control_data->basic.rise_sfx_file_id) {
                sound_sfx = plugin_ui_control_msg(control, control_data->basic.rise_sfx_file_id);
            }
            break;
        default:
            break;
        }
    }

    if (sound_sfx == NULL) {
        const char * sep = strrchr(plugin_ui_control_name(control), '_');
        if (sep) {
            plugin_ui_control_category_t category = plugin_ui_control_category_find(env, sep + 1);
            if (category) {
                switch(evt) {
                case plugin_ui_event_mouse_click:
                    sound_sfx = category->m_click_audio;
                    break;
                default:
                    break;
                }
            }
        }
    }

    if (sound_sfx) {
        if (ui_runtime_module_sound_play_by_res_path(env->m_module->m_runtime, "sfx", sound_sfx, 0) == NULL) {
            CPE_ERROR(
                env->m_module->m_em, "plugin_ui_control_play_event_sfx: control %s play sound %s fail",
                plugin_ui_control_path_dump(&env->m_module->m_dump_buffer, control), sound_sfx);
            return -1;
        }
    }

    return 0;
}

const char * plugin_ui_control_msg(plugin_ui_control_t control, uint32_t msg_id) {
    return control->m_src
        ?  ui_data_control_msg(control->m_src, msg_id)
        : "";
}
