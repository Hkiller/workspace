#include <assert.h>
#include "cpe/cfg/cfg_read.h"
#include "cpe/nm/nm_manage.h"
#include "cpe/nm/nm_read.h"
#include "gd/app/app_module.h"
#include "gd/app/app_context.h"
#include "gd/app/app_log.h"
#include "render/cache/ui_cache_manager.h"
#include "render/runtime/ui_runtime_module.h"
#include "plugin_render_ogl_module_i.h"
#include "plugin_render_ogl_cache_i.h"
#include "plugin_render_ogl_texture_i.h"
#include "plugin_render_ogl_shader_i.h"
#include "plugin_render_ogl_commit_batch_2d.h"
#include "plugin_render_ogl_commit_batch_3d.h"

static void plugin_render_ogl_module_clear(nm_node_t node);

struct nm_node_type s_nm_node_type_plugin_render_ogl_module = {
    "plugin_render_ogl_module",
    plugin_render_ogl_module_clear
};

static struct {
    const char * name; 
    int (*init)(plugin_render_ogl_module_t module);
    void (*fini)(plugin_render_ogl_module_t module);
} s_auto_reg_products[] = {
    { "ogl-capacity", plugin_render_ogl_module_init_capacity, plugin_render_ogl_module_fini_capacity }
    , { "ogl-backend", plugin_render_ogl_module_init_backend, plugin_render_ogl_module_fini_backend }
    , { "ogl-res-plugin", plugin_render_ogl_module_init_res_plugin, plugin_render_ogl_module_fini_res_plugin }
    , { "cache", plugin_render_ogl_cache_init, plugin_render_ogl_cache_fini }
    , { "batch-2d", plugin_render_ogl_batch_2d_init, plugin_render_ogl_batch_2d_fini }
    , { "batch-3d", plugin_render_ogl_batch_3d_init, plugin_render_ogl_batch_3d_fini }
};

plugin_render_ogl_module_t
plugin_render_ogl_module_create(
    gd_app_context_t app,
    ui_cache_manager_t cache_mgr, ui_runtime_module_t runtime,
    mem_allocrator_t alloc, const char * name, error_monitor_t em)
{
    struct plugin_render_ogl_module * module;
    nm_node_t module_node;
    int8_t component_pos = 0;

    assert(app);

    assert(glGetError() == 0);
    
    if (name == NULL) name = "plugin_render_ogl_module";

    module_node = nm_group_create(gd_app_nm_mgr(app), name, sizeof(struct plugin_render_ogl_module));
    if (module_node == NULL) return NULL;

    module = (plugin_render_ogl_module_t)nm_node_data(module_node);

    module->m_app = app;
    module->m_alloc = alloc;
    module->m_em = em;
    module->m_cache_mgr = cache_mgr;
    module->m_runtime = runtime;
    module->m_debug = 0;
    module->m_batch_2d = NULL;
    module->m_cache = NULL;
    
    /**/
    TAILQ_INIT(&module->m_shaders);

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

    nm_node_set_type(module_node, &s_nm_node_type_plugin_render_ogl_module);

    return module;
}

static void plugin_render_ogl_module_clear(nm_node_t node) {
    plugin_render_ogl_module_t module;
    int component_pos;

    module = (plugin_render_ogl_module_t)nm_node_data(node);

    for(component_pos = CPE_ARRAY_SIZE(s_auto_reg_products); component_pos > 0; component_pos--) {
        s_auto_reg_products[component_pos - 1].fini(module);
    }

    assert(TAILQ_EMPTY(&module->m_shaders));
}

gd_app_context_t plugin_render_ogl_module_app(plugin_render_ogl_module_t module) {
    return module->m_app;
}

void plugin_render_ogl_module_free(plugin_render_ogl_module_t module) {
    nm_node_t module_node;
    assert(module);

	module_node = nm_node_from_data(module);
    if (nm_node_type(module_node) != &s_nm_node_type_plugin_render_ogl_module) return;
    nm_node_free(module_node);
}

plugin_render_ogl_module_t
plugin_render_ogl_module_find(gd_app_context_t app, cpe_hash_string_t name) {
    nm_node_t node;

    node = nm_mgr_find_node(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_plugin_render_ogl_module) return NULL;
    return (plugin_render_ogl_module_t)nm_node_data(node);
}

plugin_render_ogl_module_t
plugin_render_ogl_module_find_nc(gd_app_context_t app, const char * name) {
    nm_node_t node;

	if (name == NULL) name = "plugin_render_ogl_module";

    node = nm_mgr_find_node_nc(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_plugin_render_ogl_module) return NULL;
    return (plugin_render_ogl_module_t)nm_node_data(node);
}

const char * plugin_render_ogl_module_name(plugin_render_ogl_module_t module) {
    return nm_node_name(nm_node_from_data(module));
}

EXPORT_DIRECTIVE
int plugin_render_ogl_module_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
    plugin_render_ogl_module_t ogl;
    ui_cache_manager_t cache_mgr;

    cache_mgr = ui_cache_manager_find_nc(app, cfg_get_string(cfg, "cache-mgr", NULL));
    if (cache_mgr == NULL) {
        APP_CTX_ERROR(app, "create plugin_barrage_module: cache-mgr not exist");
        return -1;
    }
    
    ogl =
        plugin_render_ogl_module_create(
            app,
            cache_mgr, ui_runtime_module_find_nc(app, NULL),
            gd_app_alloc(app),
            gd_app_module_name(module),
            gd_app_em(app));
    if (ogl == NULL) return -1;

    ogl->m_debug = cfg_get_int32(cfg, "debug", 0);

    if (ogl->m_debug) {
        CPE_INFO(
            gd_app_em(app), "%s: create: done",
            plugin_render_ogl_module_name(ogl));
    }

    return 0;
}

EXPORT_DIRECTIVE
void plugin_render_ogl_module_app_fini(gd_app_context_t app, gd_app_module_t module) {
    plugin_render_ogl_module_t plugin_render_ogl_module;

    plugin_render_ogl_module = plugin_render_ogl_module_find_nc(app, gd_app_module_name(module));
    if (plugin_render_ogl_module) {
        plugin_render_ogl_module_free(plugin_render_ogl_module);
    }
}

