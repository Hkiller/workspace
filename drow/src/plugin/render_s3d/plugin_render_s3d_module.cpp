#include <assert.h>
#include "cpe/cfg/cfg_read.h"
#include "cpe/nm/nm_manage.h"
#include "cpe/nm/nm_read.h"
#include "gd/app/app_module.h"
#include "gd/app/app_context.h"
#include "gd/app/app_log.h"
#include "render/cache/ui_cache_manager.h"
#include "render/runtime/ui_runtime_module.h"
#include "plugin_render_s3d_module_i.hpp"
#include "plugin_render_s3d_cache_i.hpp"
#include "plugin_render_s3d_texture_i.hpp"
#include "plugin_render_s3d_commit_batch_2d.hpp"
#include "plugin_render_s3d_commit_batch_3d.hpp"

static void plugin_render_s3d_module_clear(nm_node_t node);

struct nm_node_type s_nm_node_type_plugin_render_s3d_module = {
    "plugin_render_s3d_module",
    plugin_render_s3d_module_clear
};

static struct {
    const char * name; 
    int (*init)(plugin_render_s3d_module_t module);
    void (*fini)(plugin_render_s3d_module_t module);
} s_auto_reg_products[] = {
    { "s3d-capacity", plugin_render_s3d_module_init_capacity, plugin_render_s3d_module_fini_capacity }
    , { "s3d-cache", plugin_render_s3d_cache_init, plugin_render_s3d_cache_fini }
    , { "s3d-backend", plugin_render_s3d_module_init_backend, plugin_render_s3d_module_fini_backend }
    , { "s3d-res-plugin", plugin_render_s3d_module_init_res_plugin, plugin_render_s3d_module_fini_res_plugin }
    , { "batch-2d", plugin_render_s3d_batch_2d_init, plugin_render_s3d_batch_2d_fini }
    , { "batch-3d", plugin_render_s3d_batch_3d_init, plugin_render_s3d_batch_3d_fini }
};

plugin_render_s3d_module_t
plugin_render_s3d_module_create(
    gd_app_context_t app,
    ui_cache_manager_t cache_mgr, ui_runtime_module_t runtime,
    mem_allocrator_t alloc, const char * name, error_monitor_t em)
{
    struct plugin_render_s3d_module * module;
    nm_node_t module_node;
    size_t component_pos = 0;

    assert(app);

    if (name == NULL) name = "plugin_render_s3d_module";

    module_node = nm_group_create(gd_app_nm_mgr(app), name, sizeof(struct plugin_render_s3d_module));
    if (module_node == NULL) return NULL;

    module = (plugin_render_s3d_module_t)nm_node_data(module_node);

    module->m_app = app;
    module->m_alloc = alloc;
    module->m_em = em;
    module->m_cache_mgr = cache_mgr;
    module->m_runtime = runtime;
    module->m_debug = 0;
    module->m_cache = NULL; 
    module->m_batch_2d = NULL;

    new (&module->m_stage) flash::display::Stage();
    new (&module->m_s3d) flash::display::Stage3D();
    new (&module->m_ctx3d) flash::display3D::Context3D();
    new (&module->m_assembler) com::adobe::utils::AGALMiniAssembler();
    
    module->m_stage = internal::get_Stage();
    module->m_s3d = var(var(module->m_stage->stage3Ds)[0]);
    module->m_ctx3d = module->m_s3d->context3D;
    module->m_assembler = com::adobe::utils::AGALMiniAssembler::_new(false);
    
    for(component_pos = 0; component_pos < CPE_ARRAY_SIZE(s_auto_reg_products); ++component_pos) {
        if (s_auto_reg_products[component_pos].init(module) != 0) {
            CPE_ERROR(em, "%s: regist product %s fail!", name, s_auto_reg_products[component_pos].name);
            for(; component_pos > 0; component_pos--) {
                s_auto_reg_products[component_pos - 1].fini(module);
            }

            module->m_stage.~Stage();
            module->m_s3d.~Stage3D();
            module->m_ctx3d.~Context3D();
            module->m_assembler.~AGALMiniAssembler();
            
            nm_node_free(module_node);
            return NULL;
        }
    }

    nm_node_set_type(module_node, &s_nm_node_type_plugin_render_s3d_module);

    return module;
}

static void plugin_render_s3d_module_clear(nm_node_t node) {
    plugin_render_s3d_module_t module;
    size_t component_pos;

    module = (plugin_render_s3d_module_t)nm_node_data(node);

    plugin_render_s3d_unbind_other_textures(module, 0);
    plugin_render_s3d_unbind_other_vertexes(module, 0);

    for(component_pos = 0; component_pos < CPE_ARRAY_SIZE(s_auto_reg_products); ++component_pos) {
        s_auto_reg_products[component_pos].fini(module);
    }
    
    module->m_assembler.~AGALMiniAssembler();
    module->m_stage.~Stage();
    module->m_s3d.~Stage3D();
    module->m_ctx3d.~Context3D();
}

gd_app_context_t plugin_render_s3d_module_app(plugin_render_s3d_module_t module) {
    return module->m_app;
}

void plugin_render_s3d_module_free(plugin_render_s3d_module_t module) {
    nm_node_t module_node;
    assert(module);

	module_node = nm_node_from_data(module);
    if (nm_node_type(module_node) != &s_nm_node_type_plugin_render_s3d_module) return;
    nm_node_free(module_node);
}

plugin_render_s3d_module_t
plugin_render_s3d_module_find(gd_app_context_t app, cpe_hash_string_t name) {
    nm_node_t node;

    node = nm_mgr_find_node(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_plugin_render_s3d_module) return NULL;
    return (plugin_render_s3d_module_t)nm_node_data(node);
}

plugin_render_s3d_module_t
plugin_render_s3d_module_find_nc(gd_app_context_t app, const char * name) {
    nm_node_t node;

	if (name == NULL) name = "plugin_render_s3d_module";

    node = nm_mgr_find_node_nc(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_plugin_render_s3d_module) return NULL;
    return (plugin_render_s3d_module_t)nm_node_data(node);
}

const char * plugin_render_s3d_module_name(plugin_render_s3d_module_t module) {
    return nm_node_name(nm_node_from_data(module));
}

extern "C"
EXPORT_DIRECTIVE
int plugin_render_s3d_module_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
    plugin_render_s3d_module_t s3d;
    ui_cache_manager_t cache_mgr;

    cache_mgr = ui_cache_manager_find_nc(app, cfg_get_string(cfg, "cache-mgr", NULL));
    if (cache_mgr == NULL) {
        APP_CTX_ERROR(app, "create plugin_barrage_module: cache-mgr not exist");
        return -1;
    }
    
    s3d =
        plugin_render_s3d_module_create(
            app,
            cache_mgr, ui_runtime_module_find_nc(app, NULL),
            gd_app_alloc(app),
            gd_app_module_name(module),
            gd_app_em(app));
    if (s3d == NULL) return -1;

    s3d->m_debug = cfg_get_int32(cfg, "debug", 0);

    if (s3d->m_debug) {
        CPE_INFO(
            gd_app_em(app), "%s: create: done",
            plugin_render_s3d_module_name(s3d));
    }

    return 0;
}

extern "C"
EXPORT_DIRECTIVE
void plugin_render_s3d_module_app_fini(gd_app_context_t app, gd_app_module_t module) {
    plugin_render_s3d_module_t plugin_render_s3d_module;

    plugin_render_s3d_module = plugin_render_s3d_module_find_nc(app, gd_app_module_name(module));
    if (plugin_render_s3d_module) {
        plugin_render_s3d_module_free(plugin_render_s3d_module);
    }
}

