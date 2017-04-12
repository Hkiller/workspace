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
#include "render/model/ui_data_src.h"
#include "plugin_moving_module_i.h"
#include "plugin_moving_plan_i.h"
#include "plugin_moving_plan_track_i.h"
#include "plugin_moving_plan_point_i.h"
#include "plugin_moving_plan_node_i.h"
#include "plugin_moving_plan_segment_i.h"

extern char g_metalib_plugin_moving[];
static void plugin_moving_module_clear(nm_node_t node);

#define PLUGIN_MOVING_MODULE_LOAD_META(__arg, __name) \
    module-> __arg  = dr_lib_find_meta_by_name((LPDRMETALIB)(void*)g_metalib_plugin_moving, __name); \
    assert(module-> __arg)


struct nm_node_type s_nm_node_type_plugin_moving_module = {
    "plugin_moving_module",
    plugin_moving_module_clear
};

plugin_moving_module_t
plugin_moving_module_create(gd_app_context_t app, ui_data_mgr_t data_mgr, mem_allocrator_t alloc, const char * name, error_monitor_t em) {
    struct plugin_moving_module * module;
    nm_node_t module_node;

    assert(app);

    if (name == NULL) name = "plugin_moving_module";

    module_node = nm_group_create(gd_app_nm_mgr(app), name, sizeof(struct plugin_moving_module));
    if (module_node == NULL) return NULL;

    module = (plugin_moving_module_t)nm_node_data(module_node);

    module->m_app = app;
    module->m_alloc = alloc;
    module->m_em = em;
    module->m_debug = 0;
    module->m_data_mgr = data_mgr;

    PLUGIN_MOVING_MODULE_LOAD_META(m_meta_moving_plan, "moving_plan");
    PLUGIN_MOVING_MODULE_LOAD_META(m_meta_moving_plan_track, "moving_plan_track");
    PLUGIN_MOVING_MODULE_LOAD_META(m_meta_moving_plan_point, "moving_plan_point");
    PLUGIN_MOVING_MODULE_LOAD_META(m_meta_moving_plan_node, "moving_plan_node");
    PLUGIN_MOVING_MODULE_LOAD_META(m_meta_moving_plan_segment, "moving_plan_segment");
    
    if (ui_data_mgr_register_type(
            data_mgr, ui_data_src_type_moving_plan,
            (ui_data_product_create_fun_t)plugin_moving_plan_create, module,
            (ui_data_product_free_fun_t)plugin_moving_plan_free, module,
            NULL)
        != 0)
    {
        CPE_ERROR(em, "%s: create: register type bullets fail!", name);
        nm_node_free(module_node);
        return NULL;
    }

    TAILQ_INIT(&module->m_free_plan_tracks);
    TAILQ_INIT(&module->m_free_plan_points);
    TAILQ_INIT(&module->m_free_plan_nodes);
    TAILQ_INIT(&module->m_free_plan_segments);
    
    mem_buffer_init(&module->m_dump_buffer, module->m_alloc);

    nm_node_set_type(module_node, &s_nm_node_type_plugin_moving_module);

    return module;
}

static void plugin_moving_module_clear(nm_node_t node) {
    plugin_moving_module_t module;

    module = nm_node_data(node);

    if (ui_data_mgr_unregister_type(module->m_data_mgr, ui_data_src_type_moving_plan) != 0) {
        CPE_ERROR(module->m_em, "%s: clear: unregister type moving fail!", nm_node_name(node));
    }

    while(!TAILQ_EMPTY(&module->m_free_plan_tracks)) {
        plugin_moving_plan_track_real_free(TAILQ_FIRST(&module->m_free_plan_tracks));
    }

    while(!TAILQ_EMPTY(&module->m_free_plan_points)) {
        plugin_moving_plan_point_real_free(TAILQ_FIRST(&module->m_free_plan_points));
    }

    while(!TAILQ_EMPTY(&module->m_free_plan_nodes)) {
        plugin_moving_plan_node_real_free(TAILQ_FIRST(&module->m_free_plan_nodes));
    }

    while(!TAILQ_EMPTY(&module->m_free_plan_segments)) {
        plugin_moving_plan_segment_real_free(TAILQ_FIRST(&module->m_free_plan_segments));
    }

    mem_buffer_clear(&module->m_dump_buffer);
}

gd_app_context_t plugin_moving_module_app(plugin_moving_module_t module) {
    return module->m_app;
}

void plugin_moving_module_free(plugin_moving_module_t module) {
    nm_node_t module_node;
    assert(module);

    module_node = nm_node_from_data(module);
    if (nm_node_type(module_node) != &s_nm_node_type_plugin_moving_module) return;
    nm_node_free(module_node);
}

plugin_moving_module_t
plugin_moving_module_find(gd_app_context_t app, cpe_hash_string_t name) {
    nm_node_t node;

    node = nm_mgr_find_node(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_plugin_moving_module) return NULL;
    return (plugin_moving_module_t)nm_node_data(node);
}

plugin_moving_module_t
plugin_moving_module_find_nc(gd_app_context_t app, const char * name) {
    nm_node_t node;

    if(name == NULL) name = "plugin_moving_module";

    node = nm_mgr_find_node_nc(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_plugin_moving_module) return NULL;
    return (plugin_moving_module_t)nm_node_data(node);
}

const char * plugin_moving_module_name(plugin_moving_module_t module) {
    return nm_node_name(nm_node_from_data(module));
}

LPDRMETA plugin_moving_module_moving_plan_meta(plugin_moving_module_t module) {
    return module->m_meta_moving_plan;
}

LPDRMETA plugin_moving_module_moving_plan_track_meta(plugin_moving_module_t module) {
    return module->m_meta_moving_plan_track;
}

LPDRMETA plugin_moving_module_moving_plan_point_meta(plugin_moving_module_t module) {
    return module->m_meta_moving_plan_point;
}

LPDRMETA plugin_moving_module_moving_plan_node_meta(plugin_moving_module_t module) {
    return module->m_meta_moving_plan_node;
}

LPDRMETA plugin_moving_module_moving_plan_segment_meta(plugin_moving_module_t module) {
    return module->m_meta_moving_plan_segment;
}

ui_data_mgr_t plugin_moving_module_data_mgr(plugin_moving_module_t mgr) {
    return mgr->m_data_mgr;
}

int plugin_moving_plan_bin_save(void * ctx, ui_data_mgr_t mgr, ui_data_src_t src, const char * root, error_monitor_t em);
int plugin_moving_plan_bin_load(void * ctx, ui_data_mgr_t mgr, ui_data_src_t src, error_monitor_t em);
int plugin_moving_plan_bin_rm(void * ctx, ui_data_mgr_t mgr, ui_data_src_t src, const char * root, error_monitor_t em);

void plugin_moving_module_install_bin_loader(plugin_moving_module_t module) {
    ui_data_mgr_set_loader(
        module->m_data_mgr, ui_data_src_type_moving_plan, plugin_moving_plan_bin_load, module);
}

void plugin_moving_module_install_bin_saver(plugin_moving_module_t module) {
    ui_data_mgr_set_saver(
        module->m_data_mgr, ui_data_src_type_moving_plan, plugin_moving_plan_bin_save, plugin_moving_plan_bin_rm, module);
}

EXPORT_DIRECTIVE
int plugin_moving_module_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
    plugin_moving_module_t plugin_moving_module;
    ui_data_mgr_t data_mgr;

    data_mgr = ui_data_mgr_find_nc(app, cfg_get_string(cfg, "data-mgr", NULL));
    if (data_mgr == NULL) {
        APP_CTX_ERROR(app, "create plugin_moving_module: data-mgr not exist");
        return -1;
    }
    
    plugin_moving_module =
        plugin_moving_module_create(
            app, data_mgr,
            gd_app_alloc(app),
            gd_app_module_name(module),
            gd_app_em(app));
    if (plugin_moving_module == NULL) return -1;

    if (cfg_get_uint8(cfg, "install-bin-loader", 1)) {
        plugin_moving_module_install_bin_loader(plugin_moving_module);        
    }

    plugin_moving_module->m_debug = cfg_get_int32(cfg, "debug", 0);

    if (plugin_moving_module->m_debug) {
        CPE_INFO(
            gd_app_em(app), "%s: create: done",
            plugin_moving_module_name(plugin_moving_module));
    }

    return 0;
}

EXPORT_DIRECTIVE
void plugin_moving_module_app_fini(gd_app_context_t app, gd_app_module_t module) {
    plugin_moving_module_t plugin_moving_module;

    plugin_moving_module = plugin_moving_module_find_nc(app, gd_app_module_name(module));
    if (plugin_moving_module) {
        plugin_moving_module_free(plugin_moving_module);
    }
}
