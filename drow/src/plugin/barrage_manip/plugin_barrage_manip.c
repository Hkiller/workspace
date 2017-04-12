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
#include "plugin/barrage/plugin_barrage_module.h"
#include "plugin/barrage/plugin_barrage_data_emitter.h"
#include "plugin_barrage_manip_i.h"

static void plugin_barrage_manip_clear(nm_node_t node);

struct nm_node_type s_nm_node_type_plugin_barrage_manip = {
    "plugin_barrage_manip",
    plugin_barrage_manip_clear
};

plugin_barrage_manip_t
plugin_barrage_manip_create(
    gd_app_context_t app, ui_ed_mgr_t ed_mgr, plugin_barrage_module_t barrage_module, 
    mem_allocrator_t alloc, const char * name, error_monitor_t em)
{
    struct plugin_barrage_manip * module;
    nm_node_t module_node;

    assert(app);

    if (name == NULL) name = "plugin_barrage_manip";

    module_node = nm_group_create(gd_app_nm_mgr(app), name, sizeof(struct plugin_barrage_manip));
    if (module_node == NULL) return NULL;

    module = (plugin_barrage_manip_t)nm_node_data(module_node);

    module->m_app = app;
    module->m_alloc = alloc;
    module->m_em = em;
    module->m_debug = 0;
    module->m_ed_mgr = ed_mgr;
    module->m_barrage_module = barrage_module;

    if (ed_mgr) {
        if (ui_ed_mgr_register_src_type(ed_mgr, ui_data_src_type_barrage, plugin_barrage_emitter_ed_src_load) != 0) {
            CPE_ERROR(em, "%s: create: register type bullets fail!", name);
            nm_node_free(module_node);
            return NULL;
        }
        
        if (ui_ed_mgr_register_obj_type(
                ed_mgr, ui_data_src_type_barrage, ui_ed_obj_type_barrage,
                plugin_barrage_module_data_barrage_meta(barrage_module),
                (ui_ed_obj_delete_fun_t)plugin_barrage_data_barrage_free,
                NULL, NULL) != 0
            )
        {
            CPE_ERROR(em, "%s: create: register obj type fail!", name);
            ui_ed_mgr_unregister_src_type(ed_mgr, ui_data_src_type_barrage);
            ui_ed_mgr_unregister_obj_type(ed_mgr, ui_ed_obj_type_barrage);
            nm_node_free(module_node);
            return NULL;
        }

        if (ui_ed_mgr_register_obj_type(
                ed_mgr, ui_data_src_type_barrage, ui_ed_obj_type_barrage_emitter,
                plugin_barrage_module_data_emitter_meta(barrage_module),
                (ui_ed_obj_delete_fun_t)plugin_barrage_data_emitter_free,
                NULL, NULL) != 0
            ||
            ui_ed_mgr_register_obj_child(
                ed_mgr,
                ui_ed_obj_type_barrage,
                ui_ed_obj_type_barrage_emitter,
                plugin_barrage_emitter_emitter_ed_obj_create) != 0
            )
        {
            CPE_ERROR(em, "%s: create: register obj type fail!", name);

            ui_ed_mgr_unregister_src_type(ed_mgr, ui_data_src_type_barrage);
            ui_ed_mgr_unregister_obj_type(ed_mgr, ui_ed_obj_type_barrage_emitter);
            ui_ed_mgr_unregister_obj_type(ed_mgr, ui_ed_obj_type_barrage_emitter_trigger);
            nm_node_free(module_node);
            return NULL;
        }

        if (ui_ed_mgr_register_obj_type(
                ed_mgr, ui_data_src_type_barrage, ui_ed_obj_type_barrage_emitter_trigger,
                plugin_barrage_module_data_emitter_trigger_meta(barrage_module),
                (ui_ed_obj_delete_fun_t)plugin_barrage_data_emitter_trigger_free,
                NULL, NULL) != 0
            ||
            ui_ed_mgr_register_obj_child(
                ed_mgr,
                ui_ed_obj_type_barrage_emitter,
                ui_ed_obj_type_barrage_emitter_trigger,
                plugin_barrage_emitter_emitter_trigger_ed_obj_create) != 0
            )
        {
            CPE_ERROR(em, "%s: create: register obj type fail!", name);

            ui_ed_mgr_unregister_src_type(ed_mgr, ui_data_src_type_barrage);
            ui_ed_mgr_unregister_obj_type(ed_mgr, ui_ed_obj_type_barrage_emitter);
            ui_ed_mgr_unregister_obj_type(ed_mgr, ui_ed_obj_type_barrage_emitter_trigger);
            nm_node_free(module_node);
            return NULL;
        }

        if (ui_ed_mgr_register_obj_type(
                ed_mgr, ui_data_src_type_barrage, ui_ed_obj_type_barrage_bullet_trigger,
                plugin_barrage_module_data_bullet_trigger_meta(barrage_module),
                (ui_ed_obj_delete_fun_t)plugin_barrage_data_bullet_trigger_free,
                NULL, NULL) != 0
            ||
            ui_ed_mgr_register_obj_child(
                ed_mgr,
                ui_ed_obj_type_barrage_emitter,
                ui_ed_obj_type_barrage_bullet_trigger,
                plugin_barrage_bullet_trigger_ed_obj_create) != 0
            )
        {
            CPE_ERROR(em, "%s: create: register obj type fail!", name);

            ui_ed_mgr_unregister_src_type(ed_mgr, ui_data_src_type_barrage);
            ui_ed_mgr_unregister_obj_type(ed_mgr, ui_ed_obj_type_barrage_emitter);
            ui_ed_mgr_unregister_obj_type(ed_mgr, ui_ed_obj_type_barrage_emitter_trigger);
            ui_ed_mgr_unregister_obj_type(ed_mgr, ui_ed_obj_type_barrage_bullet_trigger);
            nm_node_free(module_node);
            return NULL;
        }
    }
    
    nm_node_set_type(module_node, &s_nm_node_type_plugin_barrage_manip);

    return module;
}

static void plugin_barrage_manip_clear(nm_node_t node) {
    plugin_barrage_manip_t module;

    module = nm_node_data(node);

    if (module->m_ed_mgr) {
        ui_ed_mgr_unregister_obj_type(module->m_ed_mgr, ui_ed_obj_type_barrage);
        ui_ed_mgr_unregister_obj_type(module->m_ed_mgr, ui_ed_obj_type_barrage_emitter);
        ui_ed_mgr_unregister_obj_type(module->m_ed_mgr, ui_ed_obj_type_barrage_emitter_trigger);
        ui_ed_mgr_unregister_obj_type(module->m_ed_mgr, ui_ed_obj_type_barrage_bullet_trigger);

        if (ui_ed_mgr_unregister_src_type(module->m_ed_mgr, ui_data_src_type_barrage) != 0) {
            CPE_ERROR(module->m_em, "%s: clear: unregister type emitter fail!", nm_node_name(node));
        }
    }
}

gd_app_context_t plugin_barrage_manip_app(plugin_barrage_manip_t module) {
    return module->m_app;
}

void plugin_barrage_manip_free(plugin_barrage_manip_t module) {
    nm_node_t module_node;
    assert(module);

    module_node = nm_node_from_data(module);
    if (nm_node_type(module_node) != &s_nm_node_type_plugin_barrage_manip) return;
    nm_node_free(module_node);
}

plugin_barrage_manip_t
plugin_barrage_manip_find(gd_app_context_t app, cpe_hash_string_t name) {
    nm_node_t node;

    node = nm_mgr_find_node(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_plugin_barrage_manip) return NULL;
    return (plugin_barrage_manip_t)nm_node_data(node);
}

plugin_barrage_manip_t
plugin_barrage_manip_find_nc(gd_app_context_t app, const char * name) {
    nm_node_t node;

    if(name == NULL) name = "plugin_barrage_manip";

    node = nm_mgr_find_node_nc(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_plugin_barrage_manip) return NULL;
    return (plugin_barrage_manip_t)nm_node_data(node);
}

const char * plugin_barrage_manip_name(plugin_barrage_manip_t module) {
    return nm_node_name(nm_node_from_data(module));
}

EXPORT_DIRECTIVE
int plugin_barrage_manip_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
    plugin_barrage_module_t barrage_module;
    plugin_barrage_manip_t plugin_barrage_manip;
    ui_ed_mgr_t ed_mgr;

    barrage_module = plugin_barrage_module_find_nc(app, cfg_get_string(cfg, "barrage-module", NULL));
    if (barrage_module == NULL) {
        APP_CTX_ERROR(app, "create plugin_barrage_manip: barrage-module not exist");
        return -1;
    }

    ed_mgr = ui_ed_mgr_find_nc(app, cfg_get_string(cfg, "ed-mgr", NULL));
    
    plugin_barrage_manip =
        plugin_barrage_manip_create(
            app, ed_mgr, barrage_module,
            gd_app_alloc(app),
            gd_app_module_name(module),
            gd_app_em(app));
    if (plugin_barrage_manip == NULL) return -1;

    plugin_barrage_manip->m_debug = cfg_get_int32(cfg, "debug", 0);

    if (plugin_barrage_manip->m_debug) {
        CPE_INFO(
            gd_app_em(app), "%s: create: done",
            plugin_barrage_manip_name(plugin_barrage_manip));
    }

    return 0;
}

EXPORT_DIRECTIVE
void plugin_barrage_manip_app_fini(gd_app_context_t app, gd_app_module_t module) {
    plugin_barrage_manip_t plugin_barrage_manip;

    plugin_barrage_manip = plugin_barrage_manip_find_nc(app, gd_app_module_name(module));
    if (plugin_barrage_manip) {
        plugin_barrage_manip_free(plugin_barrage_manip);
    }
}
