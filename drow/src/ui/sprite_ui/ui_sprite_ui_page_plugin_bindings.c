#include <assert.h>
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "plugin/ui/plugin_ui_page.h"
#include "plugin/ui/plugin_ui_page_plugin.h"
#include "plugin/ui/plugin_ui_aspect.h"
#include "plugin/ui/plugin_ui_control.h"
#include "plugin/ui/plugin_ui_control_frame.h"
#include "ui/sprite/ui_sprite_entity_calc.h"
#include "ui_sprite_ui_page_plugin_bindings_i.h"
#include "ui_sprite_ui_env_i.h"

struct ui_sprite_ui_page_plugin_bindings {
    plugin_ui_aspect_t m_aspect;
    char * m_path;
    char * m_condition;
    char * m_attr_name;
    char * m_attr_value;
};

int ui_sprite_page_plugin_bindings_init(void * ctx, plugin_ui_page_t page, plugin_ui_page_plugin_t plugin) {
    struct ui_sprite_ui_page_plugin_bindings * data = plugin_ui_page_plugin_data(plugin);
    data->m_aspect = NULL;
    data->m_path = NULL;
    data->m_condition = NULL;
    data->m_attr_name = NULL;
    data->m_attr_value = NULL;
    return 0;
}

void ui_sprite_page_plugin_bindings_fini(void * ctx, plugin_ui_page_t page, plugin_ui_page_plugin_t plugin) {
    ui_sprite_ui_env_t env = ctx;
    ui_sprite_ui_module_t module = env->m_module;
    struct ui_sprite_ui_page_plugin_bindings * data = plugin_ui_page_plugin_data(plugin);

    if(data->m_aspect) {
        plugin_ui_aspect_free(data->m_aspect);
        data->m_aspect = NULL;
    }

    if (data->m_path) mem_free(module->m_alloc, data->m_path);
    if (data->m_condition) mem_free(module->m_alloc, data->m_condition);
    if (data->m_attr_name) mem_free(module->m_alloc, data->m_attr_name);
    if (data->m_attr_value) mem_free(module->m_alloc, data->m_attr_value);
}

static int ui_sprite_page_plugin_bindings_load(void * ctx, plugin_ui_page_t page, plugin_ui_page_plugin_t plugin) {
    ui_sprite_ui_env_t env = ctx;
    ui_sprite_ui_module_t module = env->m_module;
    struct ui_sprite_ui_page_plugin_bindings * data = plugin_ui_page_plugin_data(plugin);
    plugin_ui_control_t control;
    struct plugin_ui_control_frame_it frame_it;
    plugin_ui_control_frame_t frame;
    plugin_ui_control_frame_t old_frames[100];
    uint8_t old_frame_count;
    
    if (data->m_condition) {
        uint8_t result;
        if (ui_sprite_entity_try_calc_bool(&result, data->m_condition, env->m_entity, NULL, module->m_em) != 0) {
            CPE_ERROR(
                module->m_em, "page %s: plugin bindings: calc condition %s fail!",
                plugin_ui_page_name(page), data->m_condition);
            return -1;
        }

        if (!result) return 0;
    }
    
    control = plugin_ui_control_find_child_by_path(plugin_ui_page_root_control(page), data->m_path);
    if (control == NULL) {
        CPE_ERROR(
            module->m_em, "page %s: plugin bindings: control %s not exist!",
            plugin_ui_page_name(page), data->m_path);
        return -1;
    }

    /*记录当前的frame */
    plugin_ui_control_frames(control, &frame_it);
    old_frame_count = 0;
    while((frame = plugin_ui_control_frame_it_next(&frame_it))) {
        if (old_frame_count + 1 > CPE_ARRAY_SIZE(old_frames)) {
            CPE_ERROR(
                module->m_em, "page %s: plugin bindings: control %s old frame overflow!",
                plugin_ui_page_name(page), data->m_path);
            return -1;
        }
        
        old_frames[old_frame_count++] = frame;
    }

    /*执行设置 */
    if (plugin_ui_control_set_attr_by_str(control, data->m_attr_name, data->m_attr_value) != 0) {
        CPE_ERROR(
            module->m_em, "page %s: plugin bindings: control %s set attr %s = %s fail!",
            plugin_ui_page_name(page), data->m_path, data->m_attr_name, data->m_attr_value);
        return -1;
    }

    /*新增的frame都需要收集起来 */
    plugin_ui_control_frames(control, &frame_it);
    while((frame = plugin_ui_control_frame_it_next(&frame_it))) {
        uint8_t is_old = 0;
        uint8_t i;
        for(i = 0; i < old_frame_count; ++i) {
            if (old_frames[i] == frame) {
                is_old = 1;
                break;
            }
        }

        if (is_old) continue;

        if (data->m_aspect == NULL) {
            data->m_aspect = plugin_ui_aspect_create(plugin_ui_control_env(control), NULL);
            if (data->m_aspect == NULL) {
                CPE_ERROR(
                    module->m_em, "page %s: plugin bindings: process control %s: create aspect fail!",
                    plugin_ui_page_name(page), data->m_path);
                return -1;
            }
        }

        if (plugin_ui_aspect_control_frame_add(data->m_aspect, frame, 1) != 0) {
            CPE_ERROR(
                module->m_em, "page %s: plugin bindings: add frame %s to aspect fail!",
                plugin_ui_page_name(page), plugin_ui_control_frame_dump(&module->m_dump_buffer, frame));
            plugin_ui_aspect_free(data->m_aspect);
            data->m_aspect = NULL;
            return -1;
        }
    }
    
    return 0;
}

static void ui_sprite_page_plugin_bindings_unload(void * ctx, plugin_ui_page_t page, plugin_ui_page_plugin_t plugin) {
    //ui_sprite_ui_env_t env = ctx;
    struct ui_sprite_ui_page_plugin_bindings * data = plugin_ui_page_plugin_data(plugin);

    if (data->m_aspect) {
        plugin_ui_aspect_free(data->m_aspect);
        data->m_aspect = NULL;
    }
}

static int ui_sprite_ui_env_load_page_create_binding(
    ui_sprite_ui_env_t env, plugin_ui_page_t page, const char * control_name, const char * condition, const char * name, cfg_t v)
{
    plugin_ui_page_plugin_t plugin;
    struct ui_sprite_ui_page_plugin_bindings * plugin_data;
    const char * value;

    value = cfg_as_string(v, NULL);
    if (value == NULL) {
        CPE_ERROR(
            env->m_module->m_em,
            "ui_sprite_ui_env_load_pages: page %s: control %s: plugin attr %s value format error!",
            plugin_ui_page_name(page), control_name, name);
        return -1;
    }
    
    plugin = plugin_ui_page_plugin_create(
        page, env, sizeof(struct ui_sprite_ui_page_plugin_bindings),
        ui_sprite_page_plugin_bindings_init,
        ui_sprite_page_plugin_bindings_fini,
        ui_sprite_page_plugin_bindings_load,
        ui_sprite_page_plugin_bindings_unload,
        NULL, NULL);
    if (plugin == NULL) {
        CPE_ERROR(
            env->m_module->m_em,
            "ui_sprite_ui_env_load_pages: page %s: control %s: create attr plugin attr %s ==> %s!",
            plugin_ui_page_name(page), control_name, name, value);
        return -1;
    }

    plugin_data = plugin_ui_page_plugin_data(plugin);

    plugin_data->m_path = cpe_str_mem_dup_trim(env->m_module->m_alloc, control_name);
    plugin_data->m_condition = condition ? cpe_str_mem_dup_trim(env->m_module->m_alloc, condition) : NULL;
    plugin_data->m_attr_name = cpe_str_mem_dup_trim(env->m_module->m_alloc, name);
    plugin_data->m_attr_value = cpe_str_mem_dup_trim(env->m_module->m_alloc, value);

    return 0;
}

int ui_sprite_ui_page_plugin_bindings_load(ui_sprite_ui_env_t env, plugin_ui_page_t page, cfg_t cfg) {
    const char * control_name;
    const char * condition;
    struct cfg_it attr_it;
    cfg_t attr_cfg;

    if ((control_name = cfg_get_string(cfg, "control", NULL)) == NULL) {
        CPE_ERROR(
            env->m_module->m_em, "ui_sprite_ui_env_load_pages: page %s: control not configured!",
            plugin_ui_page_name(page));
        return -1;
    }

    condition = cfg_get_string(cfg, "condition", NULL);
    
    cfg_it_init(&attr_it, cfg);
    while((attr_cfg = cfg_it_next(&attr_it))) {
        const char * name = cfg_name(attr_cfg);

        if (strcmp(name, "control") == 0
            || strcmp(name, "condition") == 0)
        {
            continue;
        }
        else {
            if (cfg_type(attr_cfg) == CPE_CFG_TYPE_STRING) {
                if (ui_sprite_ui_env_load_page_create_binding(env, page, control_name, condition, name, attr_cfg) != 0) return -1;
            }
            else if (cfg_type(attr_cfg) == CPE_CFG_TYPE_SEQUENCE) {
                struct cfg_it child_it;
                cfg_t child_cfg;

                cfg_it_init(&child_it, attr_cfg);

                if (strcmp(name, "back-res") == 0) {
                    if ((child_cfg = cfg_it_next(&child_it))) {
                        if (ui_sprite_ui_env_load_page_create_binding(env, page, control_name, condition, name, child_cfg) != 0) return -1;
                    }
                    name = "add-back-res";
                }
                else if (strcmp(name, "tail-res") == 0) {
                    if ((child_cfg = cfg_it_next(&child_it))) {
                        if (ui_sprite_ui_env_load_page_create_binding(env, page, control_name, condition, name, child_cfg) != 0) return -1;
                    }
                    name = "add-tail-res";
                }
                else if (strcmp(name, "down-res") == 0) {
                    if ((child_cfg = cfg_it_next(&child_it))) {
                        if (ui_sprite_ui_env_load_page_create_binding(env, page, control_name, condition, name, child_cfg) != 0) return -1;
                    }
                    name = "add-down-res";
                }
                
                while((child_cfg = cfg_it_next(&child_it))) {
                    if (ui_sprite_ui_env_load_page_create_binding(env, page, control_name, condition, name, child_cfg) != 0) return -1;
                }
            }
            else {
                CPE_ERROR(
                    env->m_module->m_em,
                    "ui_sprite_ui_env_load_pages: page %s: control %s: plugin attr %s type not string or sequence!",
                    plugin_ui_page_name(page), control_name, name);
                return -1;
            }
        }
    }

    return 0;
}

