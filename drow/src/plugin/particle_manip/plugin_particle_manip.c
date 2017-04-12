#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/nm/nm_manage.h"
#include "cpe/nm/nm_read.h"
#include "gd/app/app_module.h"
#include "gd/app/app_context.h"
#include "gd/app/app_log.h"
#include "render/model/ui_data_mgr.h"
#include "render/model/ui_data_src.h"
#include "render/model_ed/ui_ed_mgr.h"
#include "plugin/particle/plugin_particle_module.h"
#include "plugin/particle/plugin_particle_data.h"
#include "plugin_particle_manip_i.h"

static void plugin_particle_manip_clear(nm_node_t node);

struct nm_node_type s_nm_node_type_plugin_particle_manip = {
    "plugin_particle_manip",
    plugin_particle_manip_clear
};

plugin_particle_manip_t
plugin_particle_manip_create(
    gd_app_context_t app, ui_ed_mgr_t ed_mgr, plugin_particle_module_t particle_module, 
    mem_allocrator_t alloc, const char * name, error_monitor_t em)
{
    struct plugin_particle_manip * module;
    nm_node_t module_node;

    assert(app);

    if (name == NULL) name = "plugin_particle_manip";

    module_node = nm_group_create(gd_app_nm_mgr(app), name, sizeof(struct plugin_particle_manip));
    if (module_node == NULL) return NULL;

    module = (plugin_particle_manip_t)nm_node_data(module_node);

    module->m_app = app;
    module->m_alloc = alloc;
    module->m_em = em;
    module->m_debug = 0;
    module->m_ed_mgr = ed_mgr;
    module->m_particle_module = particle_module;

    if (ed_mgr) {
        if (ui_ed_mgr_register_src_type(ed_mgr, ui_data_src_type_particle, plugin_particle_ed_src_load) != 0) {
            CPE_ERROR(em, "%s: create: register type particle fail!", name);
            nm_node_free(module_node);
            return NULL;
        }

        if (ui_ed_mgr_register_obj_type(
                ed_mgr, ui_data_src_type_particle, ui_ed_obj_type_particle_emitter, plugin_particle_module_meta_emitter(particle_module),
                (ui_ed_obj_delete_fun_t)plugin_particle_data_emitter_free,
                NULL, NULL) != 0
            || ui_ed_mgr_register_obj_child(
                ed_mgr, ui_ed_obj_type_src, ui_ed_obj_type_particle_emitter, ui_ed_obj_create_particle_emitter) != 0
            )
            
        {
            CPE_ERROR(em, "%s: create: register ed obj particle emitter fail!", name);
            ui_ed_mgr_unregister_src_type(module->m_ed_mgr, ui_data_src_type_particle);
            nm_node_free(module_node);
            return NULL;
        }

        if (ui_ed_mgr_register_obj_type(
                ed_mgr, ui_data_src_type_particle, ui_ed_obj_type_particle_mod, plugin_particle_module_meta_mod(particle_module),
                (ui_ed_obj_delete_fun_t)plugin_particle_data_mod_free,
                NULL, NULL) != 0
            || ui_ed_mgr_register_obj_child(
                ed_mgr, ui_ed_obj_type_particle_emitter, ui_ed_obj_type_particle_mod, ui_ed_obj_create_particle_mod) != 0
            )
        {
            CPE_ERROR(em, "%s: create: register ed obj particle mod fail!", name);
            ui_ed_mgr_unregister_obj_type(module->m_ed_mgr, ui_ed_obj_type_particle_emitter);
            ui_ed_mgr_unregister_src_type(module->m_ed_mgr, ui_data_src_type_particle);
            nm_node_free(module_node);
            return NULL;
        }
    }
    
    nm_node_set_type(module_node, &s_nm_node_type_plugin_particle_manip);

    return module;
}

static void plugin_particle_manip_clear(nm_node_t node) {
    plugin_particle_manip_t module;

    module = nm_node_data(node);

    if (module->m_ed_mgr) {
        ui_ed_mgr_unregister_obj_type(module->m_ed_mgr, ui_ed_obj_type_particle_emitter);
        ui_ed_mgr_unregister_obj_type(module->m_ed_mgr, ui_ed_obj_type_particle_mod);
        ui_ed_mgr_unregister_src_type(module->m_ed_mgr, ui_data_src_type_particle);
    }
}

gd_app_context_t plugin_particle_manip_app(plugin_particle_manip_t module) {
    return module->m_app;
}

void plugin_particle_manip_free(plugin_particle_manip_t module) {
    nm_node_t module_node;
    assert(module);

    module_node = nm_node_from_data(module);
    if (nm_node_type(module_node) != &s_nm_node_type_plugin_particle_manip) return;
    nm_node_free(module_node);
}

plugin_particle_manip_t
plugin_particle_manip_find(gd_app_context_t app, cpe_hash_string_t name) {
    nm_node_t node;

    node = nm_mgr_find_node(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_plugin_particle_manip) return NULL;
    return (plugin_particle_manip_t)nm_node_data(node);
}

plugin_particle_manip_t
plugin_particle_manip_find_nc(gd_app_context_t app, const char * name) {
    nm_node_t node;

    if(name == NULL) name = "plugin_particle_manip";

    node = nm_mgr_find_node_nc(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_plugin_particle_manip) return NULL;
    return (plugin_particle_manip_t)nm_node_data(node);
}

const char * plugin_particle_manip_name(plugin_particle_manip_t module) {
    return nm_node_name(nm_node_from_data(module));
}

EXPORT_DIRECTIVE
int plugin_particle_manip_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
    plugin_particle_module_t particle_module;
    plugin_particle_manip_t plugin_particle_manip;
    ui_ed_mgr_t ed_mgr;

    particle_module = plugin_particle_module_find_nc(app, cfg_get_string(cfg, "particle-module", NULL));
    if (particle_module == NULL) {
        APP_CTX_ERROR(app, "create plugin_particle_manip: particle-module not exist");
        return -1;
    }

    ed_mgr = ui_ed_mgr_find_nc(app, cfg_get_string(cfg, "ed-mgr", NULL));
    
    plugin_particle_manip =
        plugin_particle_manip_create(
            app, ed_mgr, particle_module,
            gd_app_alloc(app), gd_app_module_name(module), gd_app_em(app));
    if (plugin_particle_manip == NULL) return -1;

    plugin_particle_manip->m_debug = cfg_get_int32(cfg, "debug", 0);

    if (plugin_particle_manip->m_debug) {
        CPE_INFO(
            gd_app_em(app), "%s: create: done",
            plugin_particle_manip_name(plugin_particle_manip));
    }

    return 0;
}

EXPORT_DIRECTIVE
void plugin_particle_manip_app_fini(gd_app_context_t app, gd_app_module_t module) {
    plugin_particle_manip_t plugin_particle_manip;

    plugin_particle_manip = plugin_particle_manip_find_nc(app, gd_app_module_name(module));
    if (plugin_particle_manip) {
        plugin_particle_manip_free(plugin_particle_manip);
    }
}
