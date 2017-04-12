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
#include "plugin/mask/plugin_mask_module.h"
#include "plugin/package_manip/plugin_package_manip.h"
#include "plugin_mask_manip_i.h"

static void plugin_mask_manip_clear(nm_node_t node);

struct nm_node_type s_nm_node_type_plugin_mask_manip = {
    "plugin_mask_manip",
    plugin_mask_manip_clear
};

static struct {
    const char * name; 
    int (*init)(plugin_mask_manip_t module);
    void (*fini)(plugin_mask_manip_t module);
} s_auto_reg_products[] = {
    { "src-convertor-module", plugin_mask_manip_src_convertor_module_regist, plugin_mask_manip_src_convertor_module_unregist }
    , { "src-convertor-sprite", plugin_mask_manip_src_convertor_sprite_regist, plugin_mask_manip_src_convertor_sprite_unregist }
    , { "src-convertor-tiledmap", plugin_mask_manip_src_convertor_tiledmap_regist, plugin_mask_manip_src_convertor_tiledmap_unregist }
};

plugin_mask_manip_t
plugin_mask_manip_create(
    gd_app_context_t app,
    plugin_package_manip_t package_manip,
    plugin_mask_module_t mask_module, 
    mem_allocrator_t alloc, const char * name, error_monitor_t em)
{
    struct plugin_mask_manip * module;
    nm_node_t module_node;
    int8_t component_pos = 0;

    assert(app);

    if (name == NULL) name = "plugin_mask_manip";

    module_node = nm_group_create(gd_app_nm_mgr(app), name, sizeof(struct plugin_mask_manip));
    if (module_node == NULL) return NULL;

    module = (plugin_mask_manip_t)nm_node_data(module_node);

    module->m_app = app;
    module->m_alloc = alloc;
    module->m_em = em;
    module->m_debug = 0;
    module->m_package_manip = package_manip;
    module->m_mask_module = mask_module;

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
    
    nm_node_set_type(module_node, &s_nm_node_type_plugin_mask_manip);

    return module;
}

static void plugin_mask_manip_clear(nm_node_t node) {
    plugin_mask_manip_t module;
    uint8_t component_pos;
    
    module = nm_node_data(node);

    for(component_pos = CPE_ARRAY_SIZE(s_auto_reg_products); component_pos > 0; component_pos--) {
        s_auto_reg_products[component_pos - 1].fini(module);
    }
}

gd_app_context_t plugin_mask_manip_app(plugin_mask_manip_t module) {
    return module->m_app;
}

void plugin_mask_manip_free(plugin_mask_manip_t module) {
    nm_node_t module_node;
    assert(module);

    module_node = nm_node_from_data(module);
    if (nm_node_type(module_node) != &s_nm_node_type_plugin_mask_manip) return;
    nm_node_free(module_node);
}

plugin_mask_manip_t
plugin_mask_manip_find(gd_app_context_t app, cpe_hash_string_t name) {
    nm_node_t node;

    node = nm_mgr_find_node(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_plugin_mask_manip) return NULL;
    return (plugin_mask_manip_t)nm_node_data(node);
}

plugin_mask_manip_t
plugin_mask_manip_find_nc(gd_app_context_t app, const char * name) {
    nm_node_t node;

    if(name == NULL) name = "plugin_mask_manip";

    node = nm_mgr_find_node_nc(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_plugin_mask_manip) return NULL;
    return (plugin_mask_manip_t)nm_node_data(node);
}

const char * plugin_mask_manip_name(plugin_mask_manip_t module) {
    return nm_node_name(nm_node_from_data(module));
}

EXPORT_DIRECTIVE
int plugin_mask_manip_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
    plugin_mask_module_t mask_module;
    plugin_mask_manip_t plugin_mask_manip;

    mask_module = plugin_mask_module_find_nc(app, cfg_get_string(cfg, "mask-module", NULL));
    if (mask_module == NULL) {
        APP_CTX_ERROR(app, "create plugin_mask_manip: mask-module not exist");
        return -1;
    }

    plugin_mask_manip =
        plugin_mask_manip_create(
            app,
            plugin_package_manip_find_nc(app, NULL),
            mask_module,
            gd_app_alloc(app),
            gd_app_module_name(module),
            gd_app_em(app));
    if (plugin_mask_manip == NULL) return -1;

    plugin_mask_manip->m_debug = cfg_get_int32(cfg, "debug", 0);

    if (plugin_mask_manip->m_debug) {
        CPE_INFO(
            gd_app_em(app), "%s: create: done",
            plugin_mask_manip_name(plugin_mask_manip));
    }

    return 0;
}

EXPORT_DIRECTIVE
void plugin_mask_manip_app_fini(gd_app_context_t app, gd_app_module_t module) {
    plugin_mask_manip_t plugin_mask_manip;

    plugin_mask_manip = plugin_mask_manip_find_nc(app, gd_app_module_name(module));
    if (plugin_mask_manip) {
        plugin_mask_manip_free(plugin_mask_manip);
    }
}
