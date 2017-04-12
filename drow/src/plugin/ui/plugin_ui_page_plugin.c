#include <assert.h>
#include "plugin/ui/plugin_ui_env_backend.h"
#include "plugin_ui_page_plugin_i.h"

plugin_ui_page_plugin_t
plugin_ui_page_plugin_create(
    plugin_ui_page_t page, void * ctx, size_t capacity,
    plugin_ui_page_plugin_on_init_t on_init,
    plugin_ui_page_plugin_on_fini_t on_fini,
    plugin_ui_page_plugin_on_load_t on_load,
    plugin_ui_page_plugin_on_unload_t on_unload,
    plugin_ui_page_plugin_on_visiable_t on_visiable,
    plugin_ui_page_plugin_on_hide_t on_hide)
{
    plugin_ui_env_t env = page->m_env;
    plugin_ui_page_plugin_t plugin;

    plugin = mem_alloc(env->m_module->m_alloc, sizeof(struct plugin_ui_page_plugin) + capacity);
    if (plugin == NULL) {
        CPE_ERROR(env->m_module->m_em, "plugin_ui_page_plugin_create: alloc fail");
        return NULL;
    }

    plugin->m_page = page;
    plugin->m_on_init = on_init;
    plugin->m_on_fini = on_fini;
    plugin->m_on_load = on_load;
    plugin->m_on_unload = on_unload;
    plugin->m_on_visiable = on_visiable;
    plugin->m_on_hide = on_hide;
    plugin->m_ctx = ctx;
    plugin->m_capacity = capacity;
    plugin->m_is_loaded = 0;
    
    TAILQ_INSERT_TAIL(&page->m_plugins, plugin, m_next_for_page);
    TAILQ_INSERT_TAIL(&env->m_page_plugins, plugin, m_next_for_env);

    if (on_init && on_init(ctx, page, plugin) != 0) {
        CPE_ERROR(env->m_module->m_em, "plugin_ui_page_plugin_create: init fail");
        TAILQ_REMOVE(&page->m_plugins, plugin, m_next_for_page);
        TAILQ_REMOVE(&page->m_env->m_page_plugins, plugin, m_next_for_env);
        mem_free(env->m_module->m_alloc, plugin);
        return NULL;
    }
        
    return plugin;
}

void plugin_ui_page_plugin_free(plugin_ui_page_plugin_t plugin) {
    plugin_ui_env_t env = plugin->m_page->m_env;

    if (plugin->m_is_loaded) {
        if (plugin->m_on_unload) plugin->m_on_unload(plugin->m_ctx, plugin->m_page, plugin);
        plugin->m_is_loaded = 0;
    }
    
    if (plugin->m_on_fini) plugin->m_on_fini(plugin->m_ctx, plugin->m_page, plugin);
                               
    TAILQ_REMOVE(&plugin->m_page->m_plugins, plugin, m_next_for_page);
    TAILQ_REMOVE(&env->m_page_plugins, plugin, m_next_for_env);
    
    mem_free(env->m_module->m_alloc, plugin);
}

plugin_ui_page_t plugin_ui_page_plugin_page(plugin_ui_page_plugin_t plugin) {
    return plugin->m_page;
}

size_t plugin_ui_page_plugin_capacity(plugin_ui_page_plugin_t plugin) {
    return plugin->m_capacity;
}

void * plugin_ui_page_plugin_data(plugin_ui_page_plugin_t plugin) {
    return plugin + 1;
}

void plugin_ui_page_plugin_clear_by_ctx_in_page(plugin_ui_page_t page, void * ctx) {
    plugin_ui_page_plugin_t plugin, next_plugin;

    for(plugin = TAILQ_FIRST(&page->m_plugins); plugin; plugin = next_plugin) {
        next_plugin = TAILQ_NEXT(plugin, m_next_for_page);

        if (plugin->m_ctx == ctx) {
            plugin_ui_page_plugin_free(plugin);
        }
    }
}

void plugin_ui_page_plugin_clear_by_ctx_in_env(plugin_ui_env_t env, void * ctx) {
}
