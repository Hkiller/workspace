#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/nm/nm_manage.h"
#include "cpe/nm/nm_read.h"
#include "gd/app/app_module.h"
#include "gd/app/app_context.h"
#include "gd/app/app_log.h"
#include "plugin/package_manip/plugin_package_manip.h"
#include "ui_sprite_manip_i.h"

static void ui_sprite_manip_clear(nm_node_t node);

struct nm_node_type s_nm_node_type_ui_sprite_manip = {
    "ui_sprite_manip",
    ui_sprite_manip_clear
};

static struct {
    const char * name; 
    int (*init)(ui_sprite_manip_t module);
    void (*fini)(ui_sprite_manip_t module);
} s_auto_reg_products[] = {
    { "res-collector-entity", ui_sprite_manip_res_collector_entity_regist, ui_sprite_manip_res_collector_entity_unregist }
    , { "res-collector-fsm", ui_sprite_manip_res_collector_fsm_regist, ui_sprite_manip_res_collector_fsm_unregist }
    , { "res-collector-scene", ui_sprite_manip_res_collector_scene_regist, ui_sprite_manip_res_collector_scene_unregist }
    , { "res-collector-page", ui_sprite_manip_res_collector_page_regist, ui_sprite_manip_res_collector_page_unregist }
    , { "res-collector-popup", ui_sprite_manip_res_collector_popup_regist, ui_sprite_manip_res_collector_popup_unregist }
};

ui_sprite_manip_t
ui_sprite_manip_create(
    gd_app_context_t app, plugin_package_manip_t package_manip,
    mem_allocrator_t alloc, const char * name, error_monitor_t em)
{
    struct ui_sprite_manip * module;
    nm_node_t module_node;
    uint8_t component_pos = 0;
    
    assert(app);

    if (name == NULL) name = "ui_sprite_manip";

    module_node = nm_group_create(gd_app_nm_mgr(app), name, sizeof(struct ui_sprite_manip));
    if (module_node == NULL) return NULL;

    module = (ui_sprite_manip_t)nm_node_data(module_node);

    module->m_app = app;
    module->m_alloc = alloc;
    module->m_em = em;
    module->m_debug = 0;
    module->m_package_manip = package_manip;

    for(component_pos = 0; component_pos < CPE_ARRAY_SIZE(s_auto_reg_products); ++component_pos) {
        if (s_auto_reg_products[component_pos].init(module) != 0) {
            CPE_ERROR(em, "%s: regist product %s fail!", name, s_auto_reg_products[component_pos].name);
            for(; component_pos > 0; component_pos--) {
                s_auto_reg_products[component_pos - 1].fini(module);
            }
            nm_node_from_data(module_node);
            return NULL;
        }
    }
    
    nm_node_set_type(module_node, &s_nm_node_type_ui_sprite_manip);

    return module;
}

static void ui_sprite_manip_clear(nm_node_t node) {
    ui_sprite_manip_t module;
    uint8_t component_pos;

    module = (ui_sprite_manip_t)nm_node_data(node);

    for(component_pos = CPE_ARRAY_SIZE(s_auto_reg_products); component_pos > 0; component_pos--) {
        s_auto_reg_products[component_pos - 1].fini(module);
    }
}

gd_app_context_t ui_sprite_manip_app(ui_sprite_manip_t module) {
    return module->m_app;
}

void ui_sprite_manip_free(ui_sprite_manip_t module) {
    nm_node_t module_node;
    assert(module);

    module_node = nm_node_from_data(module);
    if (nm_node_type(module_node) != &s_nm_node_type_ui_sprite_manip) return;
    nm_node_free(module_node);
}

ui_sprite_manip_t
ui_sprite_manip_find(gd_app_context_t app, cpe_hash_string_t name) {
    nm_node_t node;

    node = nm_mgr_find_node(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_ui_sprite_manip) return NULL;
    return (ui_sprite_manip_t)nm_node_data(node);
}

ui_sprite_manip_t
ui_sprite_manip_find_nc(gd_app_context_t app, const char * name) {
    nm_node_t node;

    if(name == NULL) name = "ui_sprite_manip";

    node = nm_mgr_find_node_nc(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_ui_sprite_manip) return NULL;
    return (ui_sprite_manip_t)nm_node_data(node);
}

const char * ui_sprite_manip_name(ui_sprite_manip_t module) {
    return nm_node_name(nm_node_from_data(module));
}

EXPORT_DIRECTIVE
int ui_sprite_manip_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
    ui_sprite_manip_t ui_sprite_manip;
    plugin_package_manip_t package_manip;

    package_manip = plugin_package_manip_find_nc(app, NULL);
    if (package_manip == NULL) {
        APP_CTX_ERROR(app, "%s: create: package manip not exist", gd_app_module_name(module));
        return -1;
    }
    
    ui_sprite_manip =
        ui_sprite_manip_create(
            app, package_manip,
            gd_app_alloc(app),
            gd_app_module_name(module),
            gd_app_em(app));
    if (ui_sprite_manip == NULL) return -1;

    ui_sprite_manip->m_debug = cfg_get_int32(cfg, "debug", 0);

    if (ui_sprite_manip->m_debug) {
        CPE_INFO(
            gd_app_em(app), "%s: create: done",
            ui_sprite_manip_name(ui_sprite_manip));
    }

    return 0;
}

EXPORT_DIRECTIVE
void ui_sprite_manip_app_fini(gd_app_context_t app, gd_app_module_t module) {
    ui_sprite_manip_t ui_sprite_manip;

    ui_sprite_manip = ui_sprite_manip_find_nc(app, gd_app_module_name(module));
    if (ui_sprite_manip) {
        ui_sprite_manip_free(ui_sprite_manip);
    }
}
