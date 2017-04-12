#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/string_utils.h"
#include "ui_cache_res_plugin_i.h"

ui_cache_res_plugin_t
ui_cache_res_plugin_create(
    ui_cache_manager_t mgr, ui_cache_res_type_t res_type,
    const char * name, uint16_t capacitiy, void * ctx,
    ui_cache_res_plugin_on_loaded_fun_t on_load,
    ui_cache_res_plugin_on_unloaded_fun_t on_unload,
    struct ui_cache_res_plugin_addition_fun * addition_funcs)
{
    ui_cache_res_plugin_t plugin;

    if (!TAILQ_EMPTY(&mgr->m_ress)) {
        CPE_ERROR(mgr->m_em, "ui_cache_res_plugin_create: already have plugin!");
        return NULL;
    }
    
    assert(res_type > 0 && (res_type - 1) < CPE_ARRAY_SIZE(mgr->m_res_plugins));
    
    plugin = mgr->m_res_plugins[res_type - 1];
    if (plugin) {
        CPE_ERROR(mgr->m_em, "ui_cache_res_plugin_create: plugin of type %d already exist!", res_type);
        return NULL;
    }

    plugin = mem_alloc(mgr->m_alloc, sizeof(struct ui_cache_res_plugin));
    if (plugin == NULL) {
        CPE_ERROR(mgr->m_em, "ui_cache_res_plugin_create: alloc fail!");
        return NULL;
    }

    plugin->m_mgr = mgr;
    plugin->m_res_type = res_type;
    cpe_str_dup(plugin->m_name, sizeof(plugin->m_name), name);
    plugin->m_capacity = capacitiy;
    plugin->m_ctx = ctx;
    plugin->m_on_load = on_load;
    plugin->m_on_unload = on_unload;
    if (addition_funcs) {
        plugin->m_addition_funcs = *addition_funcs;
    }
    else {
        bzero(&plugin->m_addition_funcs, sizeof(plugin->m_addition_funcs));
    }
    
    mgr->m_res_plugins[res_type - 1] = plugin;
    
    return plugin;
}

void ui_cache_res_plugin_free(ui_cache_res_plugin_t plugin) {
    ui_cache_manager_t mgr = plugin->m_mgr;
    ui_cache_res_t res, res_next;

    assert(plugin->m_res_type > 0 && (plugin->m_res_type - 1) < CPE_ARRAY_SIZE(mgr->m_res_plugins));
    assert(plugin == mgr->m_res_plugins[plugin->m_res_type - 1]);

    for(res = TAILQ_FIRST(&mgr->m_ress); res; res = res_next) {
        res_next = TAILQ_NEXT(res, m_next_for_mgr);
        if (res->m_res_type == plugin->m_res_type) {
            ui_cache_res_free(res);
        }
    }
    
    mgr->m_res_plugins[plugin->m_res_type - 1] = NULL;
    mem_free(mgr->m_alloc, plugin);
}

ui_cache_res_plugin_t ui_cache_res_plugin_find_by_type(ui_cache_manager_t mgr, ui_cache_res_type_t res_type) {
    ui_cache_res_plugin_t plugin;

    assert(res_type > 0 && (res_type - 1) < CPE_ARRAY_SIZE(mgr->m_res_plugins));
    
    plugin = mgr->m_res_plugins[res_type - 1];

    assert(plugin == NULL || plugin->m_res_type == res_type);

    return plugin;
}

ui_cache_res_plugin_t ui_cache_res_plugin_find_by_ctx(ui_cache_manager_t mgr, void * ctx) {
    uint8_t i;

    for(i = 0; i < CPE_ARRAY_SIZE(mgr->m_res_plugins); ++i) {
        ui_cache_res_plugin_t plugin;

        plugin = mgr->m_res_plugins[i];
        if (plugin && plugin->m_ctx == ctx) return plugin;
    }
    
    return NULL;
}

void * ui_cache_res_plugin_data(ui_cache_res_t res) {
    return res + 1;
}
