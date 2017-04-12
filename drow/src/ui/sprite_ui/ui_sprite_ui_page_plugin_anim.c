#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "render/runtime/ui_runtime_render_obj.h"
#include "plugin/ui/plugin_ui_page.h"
#include "plugin/ui/plugin_ui_page_plugin.h"
#include "ui_sprite_ui_page_plugin_anim_i.h"
#include "ui_sprite_ui_env_i.h"

struct ui_sprite_ui_page_plugin_anim {
    char * m_name;
    char * m_res;
};

static int ui_sprite_page_plugin_anim_init(void * ctx, plugin_ui_page_t page, plugin_ui_page_plugin_t plugin) {
    struct ui_sprite_ui_page_plugin_anim * data = plugin_ui_page_plugin_data(plugin);
    data->m_name = NULL;
    data->m_res = NULL;
    return 0;
}

static void ui_sprite_page_plugin_anim_fini(void * ctx, plugin_ui_page_t page, plugin_ui_page_plugin_t plugin) {
    ui_sprite_ui_env_t env = ctx;
    ui_sprite_ui_module_t module = env->m_module;
    struct ui_sprite_ui_page_plugin_anim * data = plugin_ui_page_plugin_data(plugin);
    
    if (data->m_name) mem_free(module->m_alloc, data->m_name);
    if (data->m_res) mem_free(module->m_alloc, data->m_res);
}

static int ui_sprite_page_plugin_anim_load(void * ctx, plugin_ui_page_t page, plugin_ui_page_plugin_t plugin) {
    ui_sprite_ui_env_t env = ctx;
    ui_sprite_ui_module_t module = env->m_module;
    struct ui_sprite_ui_page_plugin_anim * data = plugin_ui_page_plugin_data(plugin);
    ui_runtime_render_obj_t render_obj;

    render_obj = ui_runtime_render_obj_create_by_res(module->m_runtime, data->m_res, data->m_name);
    if (render_obj == NULL) {
        CPE_ERROR(
            module->m_em, "ui_sprite_ui_env_load_pages: page %s: animation %s create from %s fail!",
            plugin_ui_page_name(page), data->m_name, data->m_res);
        return -1;
    }

    ui_runtime_render_obj_set_auto_release(render_obj, 0);
    
    return 0;
}

static void ui_sprite_page_plugin_anim_unload(void * ctx, plugin_ui_page_t page, plugin_ui_page_plugin_t plugin) {
    ui_sprite_ui_env_t env = ctx;
    ui_sprite_ui_module_t module = env->m_module;
    struct ui_sprite_ui_page_plugin_anim * data = plugin_ui_page_plugin_data(plugin);
    ui_runtime_render_obj_t render_obj;
    
    render_obj = ui_runtime_render_obj_find(module->m_runtime, data->m_name);
    if (render_obj) {
        ui_runtime_render_obj_free(render_obj);
    }
}

int ui_sprite_ui_page_plugin_anim_create(ui_sprite_ui_env_t env, plugin_ui_page_t page, cfg_t cfg) {
    const char * name;
    const char * res;
    plugin_ui_page_plugin_t plugin;
    struct ui_sprite_ui_page_plugin_anim * plugin_data;

    cfg = cfg_child_only(cfg);
    if (cfg == NULL) {
        CPE_ERROR(
            env->m_module->m_em, "ui_sprite_ui_env_load_pages: page %s: animations format error!",
            plugin_ui_page_name(page));
        return -1;
    }
    
    name = cfg_name(cfg);
    
    res = cfg_as_string(cfg, NULL);
    if (res == NULL) {
        CPE_ERROR(
            env->m_module->m_em, "ui_sprite_ui_env_load_pages: page %s: animation %s format error!",
            plugin_ui_page_name(page), name);
        return -1;
    }

    plugin = plugin_ui_page_plugin_create(
        page, env, sizeof(struct ui_sprite_ui_page_plugin_anim),
        ui_sprite_page_plugin_anim_init,
        ui_sprite_page_plugin_anim_fini,
        ui_sprite_page_plugin_anim_load,
        ui_sprite_page_plugin_anim_unload, NULL, NULL);
    if (plugin == NULL) {
        CPE_ERROR(
            env->m_module->m_em,
            "ui_sprite_ui_env_load_pages: page %s: create anim plugin %s!",
            plugin_ui_page_name(page), name);
        return -1;
    }

    plugin_data = plugin_ui_page_plugin_data(plugin);
    plugin_data->m_name = cpe_str_mem_dup_trim(env->m_module->m_alloc, name);
    plugin_data->m_res = cpe_str_mem_dup_trim(env->m_module->m_alloc, res);

    return 0;
}

