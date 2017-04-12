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
#include "plugin/chipmunk/plugin_chipmunk_module.h"
#include "plugin/chipmunk/plugin_chipmunk_data_scene.h"
#include "plugin/chipmunk/plugin_chipmunk_data_body.h"
#include "plugin/chipmunk/plugin_chipmunk_data_fixture.h"
#include "plugin/chipmunk/plugin_chipmunk_data_polygon_node.h"
#include "plugin/chipmunk/plugin_chipmunk_data_constraint.h"
#include "plugin_chipmunk_manip_i.h"

static void plugin_chipmunk_manip_clear(nm_node_t node);

struct nm_node_type s_nm_node_type_plugin_chipmunk_manip = {
    "plugin_chipmunk_manip",
    plugin_chipmunk_manip_clear
};

plugin_chipmunk_manip_t
plugin_chipmunk_manip_create(
    gd_app_context_t app, ui_ed_mgr_t ed_mgr, plugin_chipmunk_module_t chipmunk_module, 
    mem_allocrator_t alloc, const char * name, error_monitor_t em)
{
    struct plugin_chipmunk_manip * module;
    nm_node_t module_node;

    assert(app);

    if (name == NULL) name = "plugin_chipmunk_manip";

    module_node = nm_group_create(gd_app_nm_mgr(app), name, sizeof(struct plugin_chipmunk_manip));
    if (module_node == NULL) return NULL;

    module = (plugin_chipmunk_manip_t)nm_node_data(module_node);

    module->m_app = app;
    module->m_alloc = alloc;
    module->m_em = em;
    module->m_debug = 0;
    module->m_ed_mgr = ed_mgr;
    module->m_chipmunk_module = chipmunk_module;

    if (ed_mgr) {
        if (ui_ed_mgr_register_src_type(ed_mgr, ui_data_src_type_chipmunk_scene, plugin_chipmunk_scene_ed_src_load) != 0) {
            CPE_ERROR(em, "%s: create: register type scene fail!", name);
            nm_node_free(module_node);
            return NULL;
        }

        if (ui_ed_mgr_register_obj_type(
                ed_mgr, ui_data_src_type_chipmunk_scene, ui_ed_obj_type_chipmunk_scene, plugin_chipmunk_module_data_scene_meta(chipmunk_module),
                NULL, NULL, NULL) != 0)
        {
            CPE_ERROR(em, "%s: create: register chipmunk_scene obj type fail!", name);

            ui_ed_mgr_unregister_src_type(ed_mgr, ui_data_src_type_chipmunk_scene);
            nm_node_free(module_node);
            return NULL;
        }

        if (ui_ed_mgr_register_obj_type(
                ed_mgr, ui_data_src_type_chipmunk_scene, ui_ed_obj_type_chipmunk_body, plugin_chipmunk_module_data_body_meta(chipmunk_module),
                (ui_ed_obj_delete_fun_t)plugin_chipmunk_data_body_free,
                NULL, NULL) != 0
            ||
            ui_ed_mgr_register_obj_child(
                ed_mgr, ui_ed_obj_type_chipmunk_scene, ui_ed_obj_type_chipmunk_body, plugin_chipmunk_data_body_ed_obj_create) != 0
            )
        {
            CPE_ERROR(em, "%s: create: register obj type fail!", name);

            ui_ed_mgr_unregister_src_type(ed_mgr, ui_data_src_type_chipmunk_scene);
            ui_ed_mgr_unregister_obj_type(ed_mgr, ui_ed_obj_type_chipmunk_scene);
            ui_ed_mgr_unregister_obj_type(ed_mgr, ui_ed_obj_type_chipmunk_body);
            nm_node_free(module_node);
            return NULL;
        }

        if (ui_ed_mgr_register_obj_type(
                ed_mgr, ui_data_src_type_chipmunk_scene, ui_ed_obj_type_chipmunk_fixture, plugin_chipmunk_module_data_fixture_meta(chipmunk_module),
                (ui_ed_obj_delete_fun_t)plugin_chipmunk_data_fixture_free,
                NULL, NULL) != 0
            ||
            ui_ed_mgr_register_obj_child(
                ed_mgr, ui_ed_obj_type_chipmunk_body, ui_ed_obj_type_chipmunk_fixture, plugin_chipmunk_data_fixture_ed_obj_create) != 0
            )
        {
            CPE_ERROR(em, "%s: create: register obj type fail!", name);

            ui_ed_mgr_unregister_src_type(ed_mgr, ui_data_src_type_chipmunk_scene);
            ui_ed_mgr_unregister_obj_type(ed_mgr, ui_ed_obj_type_chipmunk_scene);
            ui_ed_mgr_unregister_obj_type(ed_mgr, ui_ed_obj_type_chipmunk_body);
            ui_ed_mgr_unregister_obj_type(ed_mgr, ui_ed_obj_type_chipmunk_fixture);        
            nm_node_free(module_node);
            return NULL;
        }

        if (ui_ed_mgr_register_obj_type(
                ed_mgr, ui_data_src_type_chipmunk_scene, ui_ed_obj_type_chipmunk_polygon_node,
                plugin_chipmunk_module_data_polygon_node_meta(chipmunk_module),
                (ui_ed_obj_delete_fun_t)plugin_chipmunk_data_polygon_node_free,
                NULL, NULL) != 0
            ||
            ui_ed_mgr_register_obj_child(
                ed_mgr, ui_ed_obj_type_chipmunk_fixture, ui_ed_obj_type_chipmunk_polygon_node, plugin_chipmunk_data_polygon_node_ed_obj_create) != 0
            )
        {
            CPE_ERROR(em, "%s: create: register obj type fail!", name);

            ui_ed_mgr_unregister_src_type(ed_mgr, ui_data_src_type_chipmunk_scene);
            ui_ed_mgr_unregister_obj_type(ed_mgr, ui_ed_obj_type_chipmunk_scene);
            ui_ed_mgr_unregister_obj_type(ed_mgr, ui_ed_obj_type_chipmunk_body);        
            ui_ed_mgr_unregister_obj_type(ed_mgr, ui_ed_obj_type_chipmunk_fixture);
            ui_ed_mgr_unregister_obj_type(ed_mgr, ui_ed_obj_type_chipmunk_polygon_node);        
            nm_node_free(module_node);
            return NULL;
        }
    
        if (ui_ed_mgr_register_obj_type(
                ed_mgr, ui_data_src_type_chipmunk_scene, ui_ed_obj_type_chipmunk_constraint,
                plugin_chipmunk_module_data_constraint_meta(chipmunk_module),
                (ui_ed_obj_delete_fun_t)plugin_chipmunk_data_constraint_free,
                NULL, NULL) != 0
            ||
            ui_ed_mgr_register_obj_child(
                ed_mgr, ui_ed_obj_type_chipmunk_scene, ui_ed_obj_type_chipmunk_constraint, plugin_chipmunk_data_constraint_ed_obj_create) != 0
            )
        {
            CPE_ERROR(em, "%s: create: register obj type fail!", name);

            ui_ed_mgr_unregister_src_type(ed_mgr, ui_data_src_type_chipmunk_scene);
            ui_ed_mgr_unregister_obj_type(ed_mgr, ui_ed_obj_type_chipmunk_scene);
            ui_ed_mgr_unregister_obj_type(ed_mgr, ui_ed_obj_type_chipmunk_body);        
            ui_ed_mgr_unregister_obj_type(ed_mgr, ui_ed_obj_type_chipmunk_fixture);
            ui_ed_mgr_unregister_obj_type(ed_mgr, ui_ed_obj_type_chipmunk_polygon_node);
            ui_ed_mgr_unregister_obj_type(ed_mgr, ui_ed_obj_type_chipmunk_constraint);
        
            nm_node_free(module_node);
            return NULL;
        }
    }
    
    nm_node_set_type(module_node, &s_nm_node_type_plugin_chipmunk_manip);

    return module;
}

static void plugin_chipmunk_manip_clear(nm_node_t node) {
    plugin_chipmunk_manip_t module;

    module = nm_node_data(node);

    if (module->m_ed_mgr) {
        ui_ed_mgr_unregister_obj_type(module->m_ed_mgr, ui_ed_obj_type_chipmunk_scene);
        ui_ed_mgr_unregister_obj_type(module->m_ed_mgr, ui_ed_obj_type_chipmunk_body);
        ui_ed_mgr_unregister_obj_type(module->m_ed_mgr, ui_ed_obj_type_chipmunk_fixture);
        ui_ed_mgr_unregister_obj_type(module->m_ed_mgr, ui_ed_obj_type_chipmunk_polygon_node);

        if (ui_ed_mgr_unregister_src_type(module->m_ed_mgr, ui_data_src_type_chipmunk_scene) != 0) {
            CPE_ERROR(module->m_em, "%s: clear: unregister type scene fail!", nm_node_name(node));
        }
    }
}

gd_app_context_t plugin_chipmunk_manip_app(plugin_chipmunk_manip_t module) {
    return module->m_app;
}

void plugin_chipmunk_manip_free(plugin_chipmunk_manip_t module) {
    nm_node_t module_node;
    assert(module);

    module_node = nm_node_from_data(module);
    if (nm_node_type(module_node) != &s_nm_node_type_plugin_chipmunk_manip) return;
    nm_node_free(module_node);
}

plugin_chipmunk_manip_t
plugin_chipmunk_manip_find(gd_app_context_t app, cpe_hash_string_t name) {
    nm_node_t node;

    node = nm_mgr_find_node(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_plugin_chipmunk_manip) return NULL;
    return (plugin_chipmunk_manip_t)nm_node_data(node);
}

plugin_chipmunk_manip_t
plugin_chipmunk_manip_find_nc(gd_app_context_t app, const char * name) {
    nm_node_t node;

    if(name == NULL) name = "plugin_chipmunk_manip";

    node = nm_mgr_find_node_nc(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_plugin_chipmunk_manip) return NULL;
    return (plugin_chipmunk_manip_t)nm_node_data(node);
}

const char * plugin_chipmunk_manip_name(plugin_chipmunk_manip_t module) {
    return nm_node_name(nm_node_from_data(module));
}

int plugin_chipmunk_scene_proj_save(void * ctx, ui_data_mgr_t mgr, ui_data_src_t src, const char * root, error_monitor_t em);
int plugin_chipmunk_scene_proj_load(void * ctx, ui_data_mgr_t mgr, ui_data_src_t src, error_monitor_t em);
int plugin_chipmunk_scene_proj_rm(void * ctx, ui_data_mgr_t mgr, ui_data_src_t src, const char * root, error_monitor_t em);

void plugin_chipmunk_manip_install_proj_loader(plugin_chipmunk_manip_t manip) {
    ui_data_mgr_set_loader(
        ui_ed_mgr_data_mgr(manip->m_ed_mgr),
        ui_data_src_type_chipmunk_scene,
        plugin_chipmunk_scene_proj_load,
        manip);
}

void plugin_chipmunk_manip_install_proj_saver(plugin_chipmunk_manip_t manip) {
    ui_data_mgr_set_saver(
        ui_ed_mgr_data_mgr(manip->m_ed_mgr),
        ui_data_src_type_chipmunk_scene,
        plugin_chipmunk_scene_proj_save,
        plugin_chipmunk_scene_proj_rm,
        manip);
}

EXPORT_DIRECTIVE
int plugin_chipmunk_manip_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
    plugin_chipmunk_module_t chipmunk_module;
    plugin_chipmunk_manip_t plugin_chipmunk_manip;
    ui_ed_mgr_t ed_mgr;

    chipmunk_module = plugin_chipmunk_module_find_nc(app, cfg_get_string(cfg, "chipmunk-module", NULL));
    if (chipmunk_module == NULL) {
        APP_CTX_ERROR(app, "create plugin_chipmunk_manip: chipmunk-module not exist");
        return -1;
    }

    ed_mgr = ui_ed_mgr_find_nc(app, cfg_get_string(cfg, "ed-mgr", NULL));
    
    plugin_chipmunk_manip =
        plugin_chipmunk_manip_create(
            app, ed_mgr, chipmunk_module,
            gd_app_alloc(app),
            gd_app_module_name(module),
            gd_app_em(app));
    if (plugin_chipmunk_manip == NULL) return -1;

    plugin_chipmunk_manip->m_debug = cfg_get_int32(cfg, "debug", 0);

    if (plugin_chipmunk_manip->m_debug) {
        CPE_INFO(
            gd_app_em(app), "%s: create: done",
            plugin_chipmunk_manip_name(plugin_chipmunk_manip));
    }

    return 0;
}

EXPORT_DIRECTIVE
void plugin_chipmunk_manip_app_fini(gd_app_context_t app, gd_app_module_t module) {
    plugin_chipmunk_manip_t plugin_chipmunk_manip;

    plugin_chipmunk_manip = plugin_chipmunk_manip_find_nc(app, gd_app_module_name(module));
    if (plugin_chipmunk_manip) {
        plugin_chipmunk_manip_free(plugin_chipmunk_manip);
    }
}
