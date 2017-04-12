#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/nm/nm_manage.h"
#include "cpe/nm/nm_read.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "gd/app/app_module.h"
#include "gd/app/app_context.h"
#include "gd/app/app_log.h"
#include "render/model/ui_data_mgr.h"
#include "render/runtime/ui_runtime_module.h"
#include "plugin_tiledmap_module_i.h"
#include "plugin_tiledmap_data_scene_i.h"
#include "plugin_tiledmap_render_obj_i.h"
#include "plugin_tiledmap_render_layer_i.h"

extern char g_metalib_plugin_tiledmap[];
static void plugin_tiledmap_module_clear(nm_node_t node);
int plugin_tiledmap_data_scene_bin_save(void * ctx, ui_data_mgr_t mgr, ui_data_src_t src, const char * root, error_monitor_t em);
int plugin_tiledmap_data_scene_bin_load(void * ctx, ui_data_mgr_t mgr, ui_data_src_t src, error_monitor_t em);
int plugin_tiledmap_data_scene_bin_rm(void * ctx, ui_data_mgr_t mgr, ui_data_src_t src, const char * root, error_monitor_t em);

#define PLUGIN_TILEDMAP_MODULE_LOAD_META(__arg, __name) \
    module-> __arg  = dr_lib_find_meta_by_name((LPDRMETALIB)(void*)g_metalib_plugin_tiledmap, __name); \
    assert(module-> __arg)
    
struct nm_node_type s_nm_node_type_plugin_tiledmap_module = {
    "plugin_tiledmap_module",
    plugin_tiledmap_module_clear
};

static struct {
    const char * name; 
    int (*init)(plugin_tiledmap_module_t module);
    void (*fini)(plugin_tiledmap_module_t module);
} s_auto_reg_products[] = {
    { "tiledmap-obj", plugin_tiledmap_render_obj_regist, plugin_tiledmap_render_obj_unregist }
    , { "tiledmap-env-render", plugin_tiledmap_render_layer_regist, plugin_tiledmap_render_layer_unregist }
};

plugin_tiledmap_module_t
plugin_tiledmap_module_create(
    gd_app_context_t app, ui_data_mgr_t data_mgr, ui_runtime_module_t runtime,
    mem_allocrator_t alloc, const char * name, error_monitor_t em)
{
    struct plugin_tiledmap_module * module;
    nm_node_t module_node;
    int component_pos;

    assert(app);

    if (name == NULL) name = "plugin_tiledmap_module";

    module_node = nm_group_create(gd_app_nm_mgr(app), name, sizeof(struct plugin_tiledmap_module));
    if (module_node == NULL) return NULL;

    module = (plugin_tiledmap_module_t)nm_node_data(module_node);

    module->m_app = app;
    module->m_alloc = alloc;
    module->m_em = em;
    module->m_debug = 0;
    module->m_data_mgr = data_mgr;
    module->m_runtime = runtime;
    
    PLUGIN_TILEDMAP_MODULE_LOAD_META(m_meta_data_scene, "tiledmap_scene");
    PLUGIN_TILEDMAP_MODULE_LOAD_META(m_meta_data_layer, "tiledmap_layer");
    PLUGIN_TILEDMAP_MODULE_LOAD_META(m_meta_data_tile, "tiledmap_tile");

    if (ui_data_mgr_register_type(
            data_mgr, ui_data_src_type_tiledmap_scene,
            (ui_data_product_create_fun_t)plugin_tiledmap_data_scene_create, module,
            (ui_data_product_free_fun_t)plugin_tiledmap_data_scene_free, module,
            plugin_tiledmap_data_scene_update_usings)
        != 0)
    {
        CPE_ERROR(em, "%s: create: register type tiledmap fail!", name);
        nm_node_free(module_node);
        return NULL;
    }
    
    ui_data_mgr_set_loader(
        module->m_data_mgr, ui_data_src_type_tiledmap_scene, plugin_tiledmap_data_scene_bin_load, module);

    ui_data_mgr_set_saver(
        module->m_data_mgr, ui_data_src_type_tiledmap_scene, plugin_tiledmap_data_scene_bin_save, plugin_tiledmap_data_scene_bin_rm, module);
    
    for(component_pos = 0; component_pos < CPE_ARRAY_SIZE(s_auto_reg_products); ++component_pos) {
        if (s_auto_reg_products[component_pos].init(module) != 0) {
            CPE_ERROR(em, "%s: regist product %s fail!", name, s_auto_reg_products[component_pos].name);
            for(; component_pos > 0; component_pos--) {
                s_auto_reg_products[component_pos - 1].fini(module);
            }

            ui_data_mgr_unregister_type(module->m_data_mgr, ui_data_src_type_tiledmap_scene);
            ui_data_mgr_set_loader(module->m_data_mgr, ui_data_src_type_tiledmap_scene, NULL, NULL);
            ui_data_mgr_set_saver(module->m_data_mgr, ui_data_src_type_tiledmap_scene, NULL, NULL, NULL);
            nm_node_free(module_node);
            return NULL;
        }
    }
    
    mem_buffer_init(&module->m_dump_buffer, alloc);

    nm_node_set_type(module_node, &s_nm_node_type_plugin_tiledmap_module);

    return module;
}

static void plugin_tiledmap_module_clear(nm_node_t node) {
    plugin_tiledmap_module_t module;
    int component_pos;
    
    module = (plugin_tiledmap_module_t)nm_node_data(node);

    for(component_pos = CPE_ARRAY_SIZE(s_auto_reg_products); component_pos > 0; component_pos--) {
        s_auto_reg_products[component_pos - 1].fini(module);
    }
    
    if (ui_data_mgr_unregister_type(module->m_data_mgr, ui_data_src_type_tiledmap_scene) != 0) {
        CPE_ERROR(module->m_em, "%s: clear: unregister tiledmap scene fail!", nm_node_name(node));
    }

    ui_data_mgr_set_loader(module->m_data_mgr, ui_data_src_type_tiledmap_scene, NULL, NULL);
    ui_data_mgr_set_saver(module->m_data_mgr, ui_data_src_type_tiledmap_scene, NULL, NULL, NULL);
    
    mem_buffer_clear(&module->m_dump_buffer);
}

ui_data_mgr_t plugin_tiledmap_module_data_mgr(plugin_tiledmap_module_t mgr) {
    return mgr->m_data_mgr;
}

gd_app_context_t plugin_tiledmap_module_app(plugin_tiledmap_module_t module) {
    return module->m_app;
}

void plugin_tiledmap_module_free(plugin_tiledmap_module_t module) {
    nm_node_t module_node;
    assert(module);

    module_node = nm_node_from_data(module);
    if (nm_node_type(module_node) != &s_nm_node_type_plugin_tiledmap_module) return;
    nm_node_free(module_node);
}

plugin_tiledmap_module_t
plugin_tiledmap_module_find(gd_app_context_t app, cpe_hash_string_t name) {
    nm_node_t node;

    node = nm_mgr_find_node(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_plugin_tiledmap_module) return NULL;
    return (plugin_tiledmap_module_t)nm_node_data(node);
}

plugin_tiledmap_module_t
plugin_tiledmap_module_find_nc(gd_app_context_t app, const char * name) {
    nm_node_t node;

    if(name == NULL) name = "plugin_tiledmap_module";

    node = nm_mgr_find_node_nc(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_plugin_tiledmap_module) return NULL;
    return (plugin_tiledmap_module_t)nm_node_data(node);
}

const char * plugin_tiledmap_module_name(plugin_tiledmap_module_t module) {
    return nm_node_name(nm_node_from_data(module));
}

LPDRMETA plugin_tiledmap_module_data_scene_meta(plugin_tiledmap_module_t module) {
    return module->m_meta_data_scene;
}
    
LPDRMETA plugin_tiledmap_module_data_layer_meta(plugin_tiledmap_module_t module) {
    return module->m_meta_data_layer;
}

LPDRMETA plugin_tiledmap_module_data_tile_meta(plugin_tiledmap_module_t module) {
    return module->m_meta_data_tile;
}

EXPORT_DIRECTIVE
int plugin_tiledmap_module_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
    plugin_tiledmap_module_t plugin_tiledmap_module;
    ui_data_mgr_t data_mgr;

    data_mgr = ui_data_mgr_find_nc(app, cfg_get_string(cfg, "data-mgr", NULL));
    if (data_mgr == NULL) {
        APP_CTX_ERROR(app, "create plugin_barrage_module: data-mgr not exist");
        return -1;
    }

    plugin_tiledmap_module =
        plugin_tiledmap_module_create(
            app,
            data_mgr,
            ui_runtime_module_find_nc(app, cfg_get_string(cfg, "runtime", NULL)),
            gd_app_alloc(app),
            gd_app_module_name(module),
            gd_app_em(app));
    if (plugin_tiledmap_module == NULL) return -1;

    plugin_tiledmap_module->m_debug = cfg_get_int32(cfg, "debug", 0);

    if (plugin_tiledmap_module->m_debug) {
        CPE_INFO(
            gd_app_em(app), "%s: create: done",
            plugin_tiledmap_module_name(plugin_tiledmap_module));
    }

    return 0;
}

EXPORT_DIRECTIVE
void plugin_tiledmap_module_app_fini(gd_app_context_t app, gd_app_module_t module) {
    plugin_tiledmap_module_t plugin_tiledmap_module;

    plugin_tiledmap_module = plugin_tiledmap_module_find_nc(app, gd_app_module_name(module));
    if (plugin_tiledmap_module) {
        plugin_tiledmap_module_free(plugin_tiledmap_module);
    }
}
