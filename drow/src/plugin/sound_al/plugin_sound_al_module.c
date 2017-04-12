#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/nm/nm_manage.h"
#include "cpe/nm/nm_read.h"
#include "gd/app/app_module.h"
#include "gd/app/app_context.h"
#include "gd/app/app_log.h"
#include "render/cache/ui_cache_manager.h"
#include "render/cache/ui_cache_res.h"
#include "render/runtime/ui_runtime_module.h"
#include "plugin_sound_al_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

static void plugin_sound_al_module_clear(nm_node_t node);
    
struct nm_node_type s_nm_node_type_plugin_sound_al_module = {
    "plugin_sound_al_module",
    plugin_sound_al_module_clear
};

static struct {
    const char * name;
    int (*init)(plugin_sound_al_module_t module);
    void (*fini)(plugin_sound_al_module_t module);
} s_auto_reg_products[] = {
    { "device", plugin_sound_al_module_init_device, plugin_sound_al_module_fini_device }
    , { "context", plugin_sound_al_module_init_context, plugin_sound_al_module_fini_context }
    , { "sound-backend", plugin_sound_al_module_init_sound_backend, plugin_sound_al_module_fini_sound_backend }
    , { "backend", plugin_sound_al_module_init_backend, plugin_sound_al_module_fini_backend }
};

plugin_sound_al_module_t
plugin_sound_al_module_create(
    gd_app_context_t app, ui_cache_manager_t cache_mgr, ui_runtime_module_t runtime, 
    mem_allocrator_t alloc, const char * name, error_monitor_t em)
{
    struct plugin_sound_al_module * module;
    nm_node_t module_node;
    int8_t component_pos;

    assert(app);

    if (name == NULL) name = "plugin_sound_al_module";

    module_node = nm_group_create(gd_app_nm_mgr(app), name, sizeof(struct plugin_sound_al_module));
    if (module_node == NULL) return NULL;

    module = (plugin_sound_al_module_t)nm_node_data(module_node);

    module->m_app = app;
    module->m_alloc = alloc;
    module->m_em = em;
    module->m_debug = 0;
    module->m_cache_mgr = cache_mgr;
    module->m_runtime = runtime;
    module->m_device = NULL;
    module->m_context = NULL;

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

    nm_node_set_type(module_node, &s_nm_node_type_plugin_sound_al_module);

    return module;
}

static void plugin_sound_al_module_clear(nm_node_t node) {
    plugin_sound_al_module_t module;
    int component_pos;

    module = (plugin_sound_al_module_t)nm_node_data(node);

    for(component_pos = CPE_ARRAY_SIZE(s_auto_reg_products); component_pos > 0; component_pos--) {
        s_auto_reg_products[component_pos - 1].fini(module);
    }
    assert(module->m_device == NULL);
    assert(module->m_context == NULL);
}

void plugin_sound_al_module_free(plugin_sound_al_module_t module) {
    nm_node_t module_node;
    assert(module);

    module_node = nm_node_from_data(module);
    if (nm_node_type(module_node) != &s_nm_node_type_plugin_sound_al_module) return;
    nm_node_free(module_node);
}

plugin_sound_al_module_t
plugin_sound_al_module_find(gd_app_context_t app, cpe_hash_string_t name) {
    nm_node_t node;

    node = nm_mgr_find_node(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_plugin_sound_al_module) return NULL;
    return (plugin_sound_al_module_t)nm_node_data(node);
}

plugin_sound_al_module_t
plugin_sound_al_module_find_nc(gd_app_context_t app, const char * name) {
    nm_node_t node;

    if(name == NULL) name = "plugin_sound_al_module";

    node = nm_mgr_find_node_nc(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_plugin_sound_al_module) return NULL;
    return (plugin_sound_al_module_t)nm_node_data(node);
}

const char * plugin_sound_al_module_name(plugin_sound_al_module_t module) {
    return nm_node_name(nm_node_from_data(module));
}

const char * plugin_sound_al_module_error_str(ALenum err) {
	switch(err) {
	case AL_NO_ERROR:
		return "AL_NO_ERROR";
	case AL_INVALID_NAME:
		return "AL_INVALID_NAME";
	case AL_INVALID_ENUM:
		return "AL_INVALID_ENUM";
	case AL_INVALID_VALUE:
		return "AL_INVALID_VALUE";
	case AL_INVALID_OPERATION:
		return "AL_INVALID_OPERATION";
	case AL_OUT_OF_MEMORY:
		return "AL_OUT_OF_MEMORY";
    default:
        return "unknown al error";
	};
}

EXPORT_DIRECTIVE
int plugin_sound_al_module_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
    plugin_sound_al_module_t plugin_sound_al_module;

    plugin_sound_al_module =
        plugin_sound_al_module_create(
            app,
            ui_cache_manager_find_nc(app, NULL),
            ui_runtime_module_find_nc(app, NULL),
            gd_app_alloc(app),
            gd_app_module_name(module),
            gd_app_em(app));
    if (plugin_sound_al_module == NULL) return -1;

    plugin_sound_al_module->m_debug = cfg_get_int32(cfg, "debug", 0);

    if (plugin_sound_al_module->m_debug) {
        CPE_INFO(
            gd_app_em(app), "%s: create: done",
            plugin_sound_al_module_name(plugin_sound_al_module));
    }

    return 0;
}

EXPORT_DIRECTIVE
void plugin_sound_al_module_app_fini(gd_app_context_t app, gd_app_module_t module) {
    plugin_sound_al_module_t plugin_sound_al_module;

    plugin_sound_al_module = plugin_sound_al_module_find_nc(app, gd_app_module_name(module));
    if (plugin_sound_al_module) {
        plugin_sound_al_module_free(plugin_sound_al_module);
    }
}

#ifdef __cplusplus
}
#endif
