#include <assert.h>
#include "cpe/utils/math_ex.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/nm/nm_manage.h"
#include "cpe/nm/nm_read.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "gd/app/app_log.h"
#include "gd/app/app_module.h"
#include "gd/app/app_context.h"
#include "render/model/ui_data_mgr.h"
#include "render/cache/ui_cache_manager.h"
#include "render/runtime/ui_runtime_module.h"
#include "render/runtime/ui_runtime_render_obj_meta.h"
#include "plugin_particle_module_i.h"
#include "plugin_particle_data_i.h"
#include "plugin_particle_obj_i.h"
#include "plugin_particle_obj_emitter_i.h"
#include "plugin_particle_obj_emitter_mod_i.h"
#include "plugin_particle_obj_emitter_data_i.h"
#include "plugin_particle_obj_emitter_binding_i.h"
#include "plugin_particle_obj_particle_i.h"
#include "plugin_particle_obj_mod_data_i.h"
#include "plugin_particle_obj_plugin_i.h"

extern char g_metalib_render_model[];

#define PLUGIN_PARTICLE_MODULE_LOAD_META(__arg, __name) \
    particle_module-> __arg  = dr_lib_find_meta_by_name((LPDRMETALIB)(void*)g_metalib_render_model, __name); \
    assert(particle_module-> __arg)

static void plugin_particle_module_clear(nm_node_t node);

struct nm_node_type s_nm_node_type_plugin_particle_module = {
    "plugin_particle_module",
    plugin_particle_module_clear
};

plugin_particle_module_t
plugin_particle_module_create(
    gd_app_context_t app,
    ui_data_mgr_t data_mgr, ui_cache_manager_t cache_mgr, ui_runtime_module_t runtime,
    mem_allocrator_t alloc, const char * name, error_monitor_t em)
{
    plugin_particle_module_t particle_module;
    nm_node_t particle_module_node;

    assert(app);

    if (name == NULL) name = "plugin_particle_module";

    particle_module_node = nm_group_create(gd_app_nm_mgr(app), name, sizeof(struct plugin_particle_module));
    if (particle_module_node == NULL) return NULL;

    particle_module = (plugin_particle_module_t)nm_node_data(particle_module_node);

    particle_module->m_app = app;
    particle_module->m_alloc = alloc;
    particle_module->m_em = em;
    particle_module->m_debug = 0;
    particle_module->m_data_mgr = data_mgr;
    particle_module->m_cache_mgr = cache_mgr;
    particle_module->m_runtime = runtime;

    PLUGIN_PARTICLE_MODULE_LOAD_META(m_meta_particle_emitter, "ui_particle_emitter");
    PLUGIN_PARTICLE_MODULE_LOAD_META(m_meta_particle_mod, "ui_particle_mod");
    PLUGIN_PARTICLE_MODULE_LOAD_META(m_meta_particle_mod_data, "ui_particle_mod_data");
    PLUGIN_PARTICLE_MODULE_LOAD_META(m_meta_particle_point, "ui_curve_point");

    if (ui_data_mgr_register_type(
            data_mgr, ui_data_src_type_particle,
            (ui_data_product_create_fun_t)plugin_particle_data_create, particle_module,
            (ui_data_product_free_fun_t)plugin_particle_data_free, particle_module,
            plugin_particle_data_update_using)
        != 0)
    {
        CPE_ERROR(em, "%s: create: register type particle skeleton fail!", name);
        nm_node_free(particle_module_node);
        return NULL;
    }

    if (runtime) {
        if (ui_runtime_render_obj_meta_create(
                runtime,
                "particle",
                UI_OBJECT_TYPE_PARTICLE,
                sizeof(struct plugin_particle_obj),
                particle_module,
                plugin_particle_obj_init,
                plugin_particle_obj_set,
                NULL,
                plugin_particle_obj_update,
                plugin_particle_obj_free,
                plugin_particle_obj_render,
                plugin_particle_obj_is_playing,
                NULL,
                NULL)
            == NULL)
        {
            CPE_ERROR(em, "%s: create: register runtime obj meta fail!", name);
            ui_data_mgr_unregister_type(data_mgr, ui_data_src_type_particle);
            nm_node_free(particle_module_node);
            return NULL;
        }
    }

    ui_data_mgr_set_loader(
        particle_module->m_data_mgr, ui_data_src_type_particle, plugin_particle_data_bin_load, particle_module);

    ui_data_mgr_set_saver(
        particle_module->m_data_mgr, ui_data_src_type_particle, plugin_particle_data_bin_save, plugin_particle_data_bin_rm, particle_module);

    TAILQ_INIT(&particle_module->m_free_particles);
    TAILQ_INIT(&particle_module->m_free_mod_datas);
    TAILQ_INIT(&particle_module->m_free_emitters);
    TAILQ_INIT(&particle_module->m_free_emitter_mods);
    TAILQ_INIT(&particle_module->m_free_emitter_datas);
    TAILQ_INIT(&particle_module->m_free_emitter_bindings);
    TAILQ_INIT(&particle_module->m_free_plugins);

    mem_buffer_init(&particle_module->m_dump_buffer, alloc);
    
    nm_node_set_type(particle_module_node, &s_nm_node_type_plugin_particle_module);

    return particle_module;
}

static void plugin_particle_module_clear(nm_node_t node) {
    plugin_particle_module_t particle_module;

    particle_module = nm_node_data(node);

    while(!TAILQ_EMPTY(&particle_module->m_free_particles)) {
        plugin_particle_obj_particle_real_free(particle_module, TAILQ_FIRST(&particle_module->m_free_particles));
    }

    while(!TAILQ_EMPTY(&particle_module->m_free_mod_datas)) {
        plugin_particle_obj_mod_data_real_free(particle_module, TAILQ_FIRST(&particle_module->m_free_mod_datas));
    }

    while(!TAILQ_EMPTY(&particle_module->m_free_emitters)) {
        plugin_particle_obj_emitter_real_free(particle_module, TAILQ_FIRST(&particle_module->m_free_emitters));
    }

    while(!TAILQ_EMPTY(&particle_module->m_free_emitter_mods)) {
        plugin_particle_obj_emitter_mod_real_free(particle_module, TAILQ_FIRST(&particle_module->m_free_emitter_mods));
    }

    while(!TAILQ_EMPTY(&particle_module->m_free_emitter_datas)) {
        plugin_particle_obj_emitter_data_real_free(particle_module, TAILQ_FIRST(&particle_module->m_free_emitter_datas));
    }
    
    while(!TAILQ_EMPTY(&particle_module->m_free_emitter_bindings)) {
        plugin_particle_obj_emitter_binding_real_free(particle_module, TAILQ_FIRST(&particle_module->m_free_emitter_bindings));
    }
    
    while(!TAILQ_EMPTY(&particle_module->m_free_plugins)) {
        plugin_particle_obj_plugin_real_free(particle_module, TAILQ_FIRST(&particle_module->m_free_plugins));
    }

    if (particle_module->m_runtime) {
        ui_runtime_render_obj_meta_t obj_meta = ui_runtime_render_obj_meta_find_by_id(particle_module->m_runtime, UI_OBJECT_TYPE_PARTICLE);
        if (obj_meta) {
            ui_runtime_render_obj_meta_free(obj_meta);
        }
    }

    ui_data_mgr_unregister_type(particle_module->m_data_mgr, ui_data_src_type_particle);

    mem_buffer_clear(&particle_module->m_dump_buffer);
}

gd_app_context_t plugin_particle_module_app(plugin_particle_module_t particle_module) {
    return particle_module->m_app;
}

void plugin_particle_module_free(plugin_particle_module_t particle_module) {
    nm_node_t particle_module_node;
    assert(particle_module);

    particle_module_node = nm_node_from_data(particle_module);
    if (nm_node_type(particle_module_node) != &s_nm_node_type_plugin_particle_module) return;
    nm_node_free(particle_module_node);
}

plugin_particle_module_t
plugin_particle_module_find(gd_app_context_t app, cpe_hash_string_t name) {
    nm_node_t node;

    node = nm_mgr_find_node(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_plugin_particle_module) return NULL;
    return (plugin_particle_module_t)nm_node_data(node);
}

plugin_particle_module_t
plugin_particle_module_find_nc(gd_app_context_t app, const char * name) {
    nm_node_t node;

    if(name == NULL) name = "plugin_particle_module";

    node = nm_mgr_find_node_nc(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_plugin_particle_module) return NULL;
    return (plugin_particle_module_t)nm_node_data(node);
}

const char * plugin_particle_module_name(plugin_particle_module_t particle_module) {
    return nm_node_name(nm_node_from_data(particle_module));
}

LPDRMETA plugin_particle_module_meta_emitter(plugin_particle_module_t module) {
    return module->m_meta_particle_emitter;
}

LPDRMETA plugin_particle_module_meta_mod(plugin_particle_module_t module) {
    return module->m_meta_particle_mod;
}

LPDRMETA plugin_particle_module_meta_mod_data(plugin_particle_module_t module) {
    return module->m_meta_particle_mod_data;
}

LPDRMETA plugin_particle_module_meta_point(plugin_particle_module_t module) {
    return module->m_meta_particle_point;
}

EXPORT_DIRECTIVE
int plugin_particle_module_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
    plugin_particle_module_t plugin_particle_module;
    ui_data_mgr_t data_mgr;

    cpe_math_fast_calc_init();

    data_mgr = ui_data_mgr_find_nc(app, cfg_get_string(cfg, "data-mgr", NULL));
    if (data_mgr == NULL) {
        APP_CTX_ERROR(app, "create %s: data-mgr not exist", gd_app_module_name(module));
        return -1;
    }

    plugin_particle_module =
        plugin_particle_module_create(
            app, data_mgr,
            ui_cache_manager_find_nc(app, cfg_get_string(cfg, "cache-mgr", NULL)),
            ui_runtime_module_find_nc(app, cfg_get_string(cfg, "ui-runtime-module", NULL)),
            gd_app_alloc(app),
            gd_app_module_name(module),
            gd_app_em(app));
    if (plugin_particle_module == NULL) return -1;

    plugin_particle_module->m_debug = cfg_get_int32(cfg, "debug", 0);

    if (plugin_particle_module->m_debug) {
        CPE_INFO(
            gd_app_em(app), "%s: create: done",
            plugin_particle_module_name(plugin_particle_module));
    }

    return 0;
}

EXPORT_DIRECTIVE
void plugin_particle_module_app_fini(gd_app_context_t app, gd_app_module_t module) {
    plugin_particle_module_t plugin_particle_module;

    plugin_particle_module = plugin_particle_module_find_nc(app, gd_app_module_name(module));
    if (plugin_particle_module) {
        plugin_particle_module_free(plugin_particle_module);
    }
}
