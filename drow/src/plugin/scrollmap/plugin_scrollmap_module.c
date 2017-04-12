#include <assert.h>
#include "cpe/cfg/cfg_read.h"
#include "cpe/nm/nm_manage.h"
#include "cpe/nm/nm_read.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "gd/app/app_module.h"
#include "gd/app/app_context.h"
#include "render/runtime/ui_runtime_module.h"
#include "render/model/ui_data_mgr.h"
#include "plugin/moving/plugin_moving_module.h"
#include "plugin_scrollmap_module_i.h"
#include "plugin_scrollmap_data_tile_i.h"
#include "plugin_scrollmap_data_layer_i.h"
#include "plugin_scrollmap_data_block_i.h"
#include "plugin_scrollmap_data_script_i.h"
#include "plugin_scrollmap_env_i.h"
#include "plugin_scrollmap_render_i.h"
#include "plugin_scrollmap_data_scene_i.h"

extern char g_metalib_plugin_scrollmap[];
static void plugin_scrollmap_module_clear(nm_node_t node);

#define SCROLLMAP_LOAD_META(__arg, __name) \
    module-> __arg  = dr_lib_find_meta_by_name((LPDRMETALIB)(void*)g_metalib_plugin_scrollmap, __name); \
    assert(module-> __arg)

struct nm_node_type s_nm_node_type_scrollmap_module = {
    "scrollmap_module",
    plugin_scrollmap_module_clear
};

static struct {
    const char * name; 
    int (*init)(plugin_scrollmap_module_t module);
    void (*fini)(plugin_scrollmap_module_t module);
} s_auto_reg_products[] = {
    { "scrollmap-data-scene", plugin_scrollmap_data_scene_regist, plugin_scrollmap_data_scene_unregist }
    , { "scrollmap-env-render", plugin_scrollmap_render_regist, plugin_scrollmap_render_unregist }
};

plugin_scrollmap_module_t
plugin_scrollmap_module_create(
    gd_app_context_t app,
    ui_data_mgr_t data_mgr, ui_runtime_module_t runtime,
    plugin_moving_module_t moving_module,
    mem_allocrator_t alloc, const char * name, error_monitor_t em)
{
    struct plugin_scrollmap_module * module;
    nm_node_t module_node;
    int8_t component_pos = 0;

    assert(app);

    if (name == NULL) name = "plugin_scrollmap_module";

    module_node = nm_group_create(gd_app_nm_mgr(app), name, sizeof(struct plugin_scrollmap_module));
    if (module_node == NULL) return NULL;

    module = (plugin_scrollmap_module_t)nm_node_data(module_node);

    module->m_app = app;
    module->m_alloc = alloc;
    module->m_em = em;
    module->m_data_mgr = data_mgr;
    module->m_runtime = runtime;
    module->m_moving_module = moving_module;
    module->m_debug = 0;
    TAILQ_INIT(&module->m_free_data_tiles);
    TAILQ_INIT(&module->m_free_data_layers);
    TAILQ_INIT(&module->m_free_data_blocks);
    TAILQ_INIT(&module->m_free_data_scripts);
    
    SCROLLMAP_LOAD_META(m_meta_tile, "scrollmap_tile");
    SCROLLMAP_LOAD_META(m_meta_layer, "scrollmap_layer");
    SCROLLMAP_LOAD_META(m_meta_block, "scrollmap_block");
    SCROLLMAP_LOAD_META(m_meta_script, "scrollmap_script");

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

    nm_node_set_type(module_node, &s_nm_node_type_scrollmap_module);

    return module;
}

static void plugin_scrollmap_module_clear(nm_node_t node) {
    plugin_scrollmap_module_t module;
    int component_pos;

    module = (plugin_scrollmap_module_t)nm_node_data(node);

    for(component_pos = CPE_ARRAY_SIZE(s_auto_reg_products); component_pos > 0; component_pos--) {
        s_auto_reg_products[component_pos - 1].fini(module);
    }

    while(!TAILQ_EMPTY(&module->m_free_data_tiles)) {
        plugin_scrollmap_data_tile_real_free(TAILQ_FIRST(&module->m_free_data_tiles));
    }
    
    while(!TAILQ_EMPTY(&module->m_free_data_layers)) {
        plugin_scrollmap_data_layer_real_free(TAILQ_FIRST(&module->m_free_data_layers));
    }
    
    while(!TAILQ_EMPTY(&module->m_free_data_blocks)) {
        plugin_scrollmap_data_block_real_free(TAILQ_FIRST(&module->m_free_data_blocks));
    }

    while(!TAILQ_EMPTY(&module->m_free_data_scripts)) {
        plugin_scrollmap_data_script_real_free(TAILQ_FIRST(&module->m_free_data_scripts));
    }
}

gd_app_context_t plugin_scrollmap_module_app(plugin_scrollmap_module_t module) {
    return module->m_app;
}

void plugin_scrollmap_module_free(plugin_scrollmap_module_t module) {
    nm_node_t module_node;
    assert(module);

	module_node = nm_node_from_data(module);
    if (nm_node_type(module_node) != &s_nm_node_type_scrollmap_module) return;
    nm_node_free(module_node);
}

plugin_scrollmap_module_t
plugin_scrollmap_module_find(gd_app_context_t app, cpe_hash_string_t name) {
    nm_node_t node;

    node = nm_mgr_find_node(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_scrollmap_module) return NULL;
    return (plugin_scrollmap_module_t)nm_node_data(node);
}

plugin_scrollmap_module_t
plugin_scrollmap_module_find_nc(gd_app_context_t app, const char * name) {
    nm_node_t node;

	if (name == NULL) name = "plugin_scrollmap_module";

    node = nm_mgr_find_node_nc(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_scrollmap_module) return NULL;
    return (plugin_scrollmap_module_t)nm_node_data(node);
}

const char * plugin_scrollmap_module_name(plugin_scrollmap_module_t module) {
    return nm_node_name(nm_node_from_data(module));
}

LPDRMETA plugin_scrollmap_module_tile_meta(plugin_scrollmap_module_t module) {
    return module->m_meta_tile;
}

LPDRMETA plugin_scrollmap_module_layer_meta(plugin_scrollmap_module_t module) {
    return module->m_meta_layer;
}

LPDRMETA plugin_scrollmap_module_block_meta(plugin_scrollmap_module_t module) {
    return module->m_meta_block;
}

LPDRMETA plugin_scrollmap_module_script_meta(plugin_scrollmap_module_t module) {
    return module->m_meta_script;
}

EXPORT_DIRECTIVE
int plugin_scrollmap_module_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
    plugin_scrollmap_module_t plugin_scrollmap_module;

    plugin_scrollmap_module =
        plugin_scrollmap_module_create(
            app,
            ui_data_mgr_find_nc(app, cfg_get_string(cfg, "runtime", NULL)),
            ui_runtime_module_find_nc(app, cfg_get_string(cfg, "runtime", NULL)),
            plugin_moving_module_find_nc(app, NULL),
            gd_app_alloc(app),
            gd_app_module_name(module),
            gd_app_em(app));
    if (plugin_scrollmap_module == NULL) return -1;

    plugin_scrollmap_module->m_debug = cfg_get_int32(cfg, "debug", 0);

    if (plugin_scrollmap_module->m_debug) {
        CPE_INFO(
            gd_app_em(app), "%s: create: done",
            plugin_scrollmap_module_name(plugin_scrollmap_module));
    }

    return 0;
}

EXPORT_DIRECTIVE
void plugin_scrollmap_module_app_fini(gd_app_context_t app, gd_app_module_t module) {
    plugin_scrollmap_module_t plugin_scrollmap_module;

    plugin_scrollmap_module = plugin_scrollmap_module_find_nc(app, gd_app_module_name(module));
    if (plugin_scrollmap_module) {
        plugin_scrollmap_module_free(plugin_scrollmap_module);
    }
}

