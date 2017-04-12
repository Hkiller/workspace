#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/nm/nm_manage.h"
#include "cpe/nm/nm_read.h"
#include "gd/app/app_module.h"
#include "gd/app/app_context.h"
#include "gd/app/app_log.h"
#include "plugin/layout/plugin_layout_render.h"
#include "plugin/layout/plugin_layout_animation.h"
#include "plugin/layout/plugin_layout_animation_selection.h"
#include "plugin_editor_module_i.h"
#include "plugin_editor_editing_i.h"

#ifdef __cplusplus
extern "C" {
#endif

static void plugin_editor_module_clear(nm_node_t node);
    
struct nm_node_type s_nm_node_type_plugin_editor_module = {
    "plugin_editor_module",
    plugin_editor_module_clear
};

static struct {
    const char * name;
    int (*init)(plugin_editor_module_t module);
    void (*fini)(plugin_editor_module_t module);
} s_auto_reg_products[] = {
    { "editing", plugin_editor_editing_register, plugin_editor_editing_unregister }
    , { "backend", plugin_editor_backend_init, plugin_editor_backend_fini }
};

plugin_editor_module_t
plugin_editor_module_create(
    gd_app_context_t app, mem_allocrator_t alloc,
    const char * name, error_monitor_t em)
{
    struct plugin_editor_module * module;
    nm_node_t module_node;
    int8_t component_pos;

    assert(app);

    if (name == NULL) name = "plugin_editor_module";

    module_node = nm_group_create(gd_app_nm_mgr(app), name, sizeof(struct plugin_editor_module));
    if (module_node == NULL) return NULL;

    module = (plugin_editor_module_t)nm_node_data(module_node);

    module->m_app = app;
    module->m_alloc = alloc;
    module->m_em = em;
    module->m_debug = 0;
    module->m_animation_meta_editing = NULL;
    TAILQ_INIT(&module->m_editings);
    module->m_active_editing = NULL;
    
    for(component_pos = 0; component_pos < CPE_ARRAY_SIZE(s_auto_reg_products); ++component_pos) {
        if (s_auto_reg_products[component_pos].init(module) != 0) {
            CPE_ERROR(em, "%s: regist product %s fail!", name, s_auto_reg_products[component_pos].name);
            for(; component_pos > 0; component_pos--) {
                s_auto_reg_products[component_pos - 1].fini(module);
            }
            nm_node_free(module_node);
            return NULL;
        }
    }

    nm_node_set_type(module_node, &s_nm_node_type_plugin_editor_module);

    return module;
}

static void plugin_editor_module_clear(nm_node_t node) {
    plugin_editor_module_t module;
    int component_pos;

    module = (plugin_editor_module_t)nm_node_data(node);

    while(!TAILQ_EMPTY(&module->m_editings)) {
        plugin_editor_editing_free(TAILQ_FIRST(&module->m_editings));
    }
    assert(module->m_active_editing == NULL);
    
    for(component_pos = CPE_ARRAY_SIZE(s_auto_reg_products); component_pos > 0; component_pos--) {
        s_auto_reg_products[component_pos - 1].fini(module);
    }

    assert(module->m_animation_meta_editing == NULL);
}

void plugin_editor_module_free(plugin_editor_module_t module) {
    nm_node_t module_node;
    assert(module);

    module_node = nm_node_from_data(module);
    if (nm_node_type(module_node) != &s_nm_node_type_plugin_editor_module) return;
    nm_node_free(module_node);
}

plugin_editor_module_t
plugin_editor_module_find(gd_app_context_t app, cpe_hash_string_t name) {
    nm_node_t node;

    node = nm_mgr_find_node(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_plugin_editor_module) return NULL;
    return (plugin_editor_module_t)nm_node_data(node);
}

plugin_editor_module_t
plugin_editor_module_find_nc(gd_app_context_t app, const char * name) {
    nm_node_t node;

    if(name == NULL) name = "plugin_editor_module";

    node = nm_mgr_find_node_nc(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_plugin_editor_module) return NULL;
    return (plugin_editor_module_t)nm_node_data(node);
}

const char * plugin_editor_module_name(plugin_editor_module_t module) {
    return nm_node_name(nm_node_from_data(module));
}

plugin_editor_editing_t
plugin_editor_module_active_editing(plugin_editor_module_t module) {
    return module->m_active_editing;
}

void plugin_editor_module_set_active_editing(plugin_editor_module_t module, plugin_editor_editing_t editing) {
    if (module->m_active_editing == editing) return;

    if (editing) {
        plugin_layout_render_t render = plugin_layout_animation_render(plugin_layout_animation_from_data(editing));
        
        module->m_active_editing = editing;
        
        plugin_editor_backend_update_content(module, render);
        plugin_editor_backend_update_selection(module, render);
    }
    else {
        if (module->m_active_editing) {
            plugin_editor_backend_close(module);
            module->m_active_editing = NULL;
        }
    }
}

int plugin_editor_module_copy_to_clipboard(plugin_editor_module_t module, plugin_layout_render_t render) {
    plugin_editor_editing_t editing;
    
    if ((editing = plugin_editor_editing_find_first(render))) {
        return plugin_editor_editing_copy_to_clipboard(editing);
    }
    else {
        char * text_utf8;
        int rv;
        
        text_utf8 = plugin_layout_render_text_utf8(module->m_alloc, render);
        if (text_utf8 == NULL) return -1;

        rv = plugin_editor_backend_clipboard_put(module, text_utf8);
        
        mem_free(module->m_alloc, text_utf8);

        return rv == 0 ? 0 : -1;
    }
}
    
int plugin_editor_module_past_from_clipboard(plugin_editor_module_t module, plugin_layout_render_t render) {
    plugin_editor_editing_t editing;
    
    if ((editing = plugin_editor_editing_find_first(render))) {
        return plugin_editor_editing_past_from_clipboard(editing);
    }
    else {
        char * text_utf8;
        int rv;
        
        text_utf8 = plugin_editor_backend_clipboard_get(module);
        if (text_utf8 == NULL) return -1;

        rv = plugin_layout_render_set_data(render, text_utf8);
        
        mem_free(module->m_alloc, text_utf8);

        return rv == 0 ? 0 : -1;
    }
}

EXPORT_DIRECTIVE
int plugin_editor_module_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
    plugin_editor_module_t plugin_editor_module;
    
    plugin_editor_module =
        plugin_editor_module_create(
            app, gd_app_alloc(app),
            gd_app_module_name(module), gd_app_em(app));
    if (plugin_editor_module == NULL) return -1;
    
    plugin_editor_module->m_debug = cfg_get_int32(cfg, "debug", 0);

    if (plugin_editor_module->m_debug) {
        CPE_INFO(
            gd_app_em(app), "%s: create: done",
            plugin_editor_module_name(plugin_editor_module));
    }

    return 0;
}

EXPORT_DIRECTIVE
void plugin_editor_module_app_fini(gd_app_context_t app, gd_app_module_t module) {
    plugin_editor_module_t plugin_editor_module;

    plugin_editor_module = plugin_editor_module_find_nc(app, gd_app_module_name(module));
    if (plugin_editor_module) {
        plugin_editor_module_free(plugin_editor_module);
    }
}

#ifdef __cplusplus
}
#endif
