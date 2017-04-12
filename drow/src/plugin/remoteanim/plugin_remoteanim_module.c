#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "cpe/utils/string_utils.h"
#include "cpe/utils/math_ex.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/nm/nm_manage.h"
#include "cpe/nm/nm_read.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "gd/app/app_module.h"
#include "gd/app/app_context.h"
#include "gd/app/app_log.h"
#include "gd/net_trans/net_trans_manage.h"
#include "render/model/ui_data_mgr.h"
#include "render/runtime/ui_runtime_module.h"
#include "plugin_remoteanim_module_i.h"
#include "plugin_remoteanim_obj_i.h"
#include "plugin_remoteanim_group_i.h"
#include "plugin_remoteanim_block_i.h"

#ifdef __cplusplus
extern "C" {
#endif

static void plugin_remoteanim_module_clear(nm_node_t node);

#define PLUGIN_CHIPMUNK_MODULE_LOAD_META(__arg, __name) \
    module-> __arg  = dr_lib_find_meta_by_name((LPDRMETALIB)(void*)g_metalib_plugin_remoteanim, __name); \
    assert(module-> __arg)
    
struct nm_node_type s_nm_node_type_plugin_remoteanim_module = {
    "plugin_remoteanim_module",
    plugin_remoteanim_module_clear
};

static struct {
    const char * name; 
    int (*init)(plugin_remoteanim_module_t module);
    void (*fini)(plugin_remoteanim_module_t module);
} s_auto_reg_products[] = {
    { "plugin-remoteanim-obj", plugin_remoteanim_obj_register, plugin_remoteanim_obj_unregister }
};
    
plugin_remoteanim_module_t
plugin_remoteanim_module_create(
    gd_app_context_t app,
    net_trans_manage_t trans_mgr, ui_runtime_module_t runtime,
    mem_allocrator_t alloc, const char * name, error_monitor_t em)
{
    struct plugin_remoteanim_module * module;
    nm_node_t module_node;
    int8_t component_pos;

    assert(app);

    if (name == NULL) name = "plugin_remoteanim_module";

    module_node = nm_group_create(gd_app_nm_mgr(app), name, sizeof(struct plugin_remoteanim_module));
    if (module_node == NULL) return NULL;

    module = (plugin_remoteanim_module_t)nm_node_data(module_node);

    module->m_app = app;
    module->m_alloc = alloc;
    module->m_em = em;
    module->m_debug = 0;
    module->m_trans_mgr = trans_mgr;
    module->m_runtime = runtime;
    TAILQ_INIT(&module->m_free_blocks);
    
    if (cpe_hash_table_init(
            &module->m_groups,
            module->m_alloc,
            (cpe_hash_fun_t) plugin_remoteanim_group_hash,
            (cpe_hash_eq_t) plugin_remoteanim_group_eq,
            CPE_HASH_OBJ2ENTRY(plugin_remoteanim_group, m_hh),
            -1) != 0)
    {
        nm_node_free(module_node);
        return NULL;
    }

    if (cpe_hash_table_init(
            &module->m_blocks,
            module->m_alloc,
            (cpe_hash_fun_t) plugin_remoteanim_block_hash,
            (cpe_hash_eq_t) plugin_remoteanim_block_eq,
            CPE_HASH_OBJ2ENTRY(plugin_remoteanim_block, m_hh),
            -1) != 0)
    {
        cpe_hash_table_fini(&module->m_groups);
        nm_node_free(module_node);
        return NULL;
    }
    
    for(component_pos = 0; component_pos < CPE_ARRAY_SIZE(s_auto_reg_products); ++component_pos) {
        if (s_auto_reg_products[component_pos].init(module) != 0) {
            CPE_ERROR(em, "%s: regist product %s fail!", name, s_auto_reg_products[component_pos].name);
            for(; component_pos > 0; component_pos--) {
                s_auto_reg_products[component_pos - 1].fini(module);
            }

            cpe_hash_table_fini(&module->m_groups);
            nm_node_free(module_node);
            return NULL;
        }
    }
    
    nm_node_set_type(module_node, &s_nm_node_type_plugin_remoteanim_module);

    return module;
}

static void plugin_remoteanim_module_clear(nm_node_t node) {
    plugin_remoteanim_module_t module;
    int component_pos;

    module = (plugin_remoteanim_module_t)nm_node_data(node);

    for(component_pos = CPE_ARRAY_SIZE(s_auto_reg_products); component_pos > 0; component_pos--) {
        s_auto_reg_products[component_pos - 1].fini(module);
    }

    while(!TAILQ_EMPTY(&module->m_free_blocks)) {
        plugin_remoteanim_block_real_free(TAILQ_FIRST(&module->m_free_blocks));
    }
}
    
gd_app_context_t plugin_remoteanim_module_app(plugin_remoteanim_module_t module) {
    return module->m_app;
}

void plugin_remoteanim_module_free(plugin_remoteanim_module_t module) {
    nm_node_t module_node;
    assert(module);

    module_node = nm_node_from_data(module);
    if (nm_node_type(module_node) != &s_nm_node_type_plugin_remoteanim_module) return;
    nm_node_free(module_node);
}

plugin_remoteanim_module_t
plugin_remoteanim_module_find(gd_app_context_t app, cpe_hash_string_t name) {
    nm_node_t node;

    node = nm_mgr_find_node(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_plugin_remoteanim_module) return NULL;
    return (plugin_remoteanim_module_t)nm_node_data(node);
}

plugin_remoteanim_module_t
plugin_remoteanim_module_find_nc(gd_app_context_t app, const char * name) {
    nm_node_t node;

    if(name == NULL) name = "plugin_remoteanim_module";

    node = nm_mgr_find_node_nc(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_plugin_remoteanim_module) return NULL;
    return (plugin_remoteanim_module_t)nm_node_data(node);
}

const char * plugin_remoteanim_module_name(plugin_remoteanim_module_t module) {
    return nm_node_name(nm_node_from_data(module));
}
    
EXPORT_DIRECTIVE
int plugin_remoteanim_module_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
    plugin_remoteanim_module_t plugin_remoteanim_module;
    struct cfg_it group_it;
    cfg_t group_cfg;
    
    plugin_remoteanim_module =
        plugin_remoteanim_module_create(
            app,
            net_trans_manage_find_nc(app, NULL),
            ui_runtime_module_find_nc(app, NULL),
            gd_app_alloc(app),
            gd_app_module_name(module),
            gd_app_em(app));
    if (plugin_remoteanim_module == NULL) return -1;

    plugin_remoteanim_module->m_debug = cfg_get_int32(cfg, "debug", 0);

    cfg_it_init(&group_it, cfg_find_cfg(cfg, "groups"));
    while((group_cfg = cfg_it_next(&group_it))) {
        const char * name = cfg_get_string(group_cfg, "name", NULL);
        ui_vector_2 capacity = UI_VECTOR_2_INITLIZER(
            cfg_get_uint32(group_cfg, "capacity.width", 0),
            cfg_get_uint32(group_cfg, "capacity.height", 0));
        plugin_remoteanim_group_t group;
        
        if (name == NULL) {
            APP_CTX_ERROR(app, "plugin_remoteanim_module_app_init: group name not configured!");
            plugin_remoteanim_module_free(plugin_remoteanim_module);
            return -1;
        }

        if (capacity.x <= 0.0f || capacity.y <= 0.0f || !cpe_math_32_is_pow2((uint32_t)capacity.x) || !cpe_math_32_is_pow2((uint32_t)capacity.y)) {
            APP_CTX_ERROR(app, "plugin_remoteanim_module_app_init: group %s capacity (%fx%f) error!", name, capacity.x, capacity.y);
            plugin_remoteanim_module_free(plugin_remoteanim_module);
            return -1;
        }

        group = plugin_remoteanim_group_create(plugin_remoteanim_module, name, &capacity);
        if (group == NULL) {
            APP_CTX_ERROR(app, "plugin_remoteanim_module_app_init: group %s create error!", name);
            plugin_remoteanim_module_free(plugin_remoteanim_module);
            return -1;
        }

        group->m_auto_free = cfg_get_uint8(group_cfg, "auto-free", group->m_auto_free);
    }
    
    if (plugin_remoteanim_module->m_debug) {
        CPE_INFO(
            gd_app_em(app), "%s: create: done",
            plugin_remoteanim_module_name(plugin_remoteanim_module));
    }

    return 0;
}

EXPORT_DIRECTIVE
void plugin_remoteanim_module_app_fini(gd_app_context_t app, gd_app_module_t module) {
    plugin_remoteanim_module_t plugin_remoteanim_module;

    plugin_remoteanim_module = plugin_remoteanim_module_find_nc(app, gd_app_module_name(module));
    if (plugin_remoteanim_module) {
        plugin_remoteanim_module_free(plugin_remoteanim_module);
    }
}

#ifdef __cplusplus
}
#endif
