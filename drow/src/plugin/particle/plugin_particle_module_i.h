#ifndef UI_PLUGIN_PARTICLE_MODULE_I_H
#define UI_PLUGIN_PARTICLE_MODULE_I_H
#include "cpe/pal/pal_queue.h"
#include "cpe/utils/hash.h"
#include "cpe/utils/buffer.h"
#include "plugin/particle/plugin_particle_module.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct plugin_particle_obj_emitter_mod * plugin_particle_obj_emitter_mod_t;
typedef struct plugin_particle_obj_emitter_data * plugin_particle_obj_emitter_data_t;
typedef struct plugin_particle_obj_mod_data * plugin_particle_obj_mod_data_t;
typedef struct plugin_particle_obj_emitter_binding * plugin_particle_obj_emitter_binding_t;
typedef struct plugin_particle_obj_emitter_runtime * plugin_particle_obj_emitter_runtime_t;

typedef TAILQ_HEAD(plugin_particle_obj_emitter_list, plugin_particle_obj_emitter) plugin_particle_obj_emitter_list_t;
typedef TAILQ_HEAD(plugin_particle_obj_emitter_mod_list, plugin_particle_obj_emitter_mod) plugin_particle_obj_emitter_mod_list_t;
typedef TAILQ_HEAD(plugin_particle_obj_emitter_data_list, plugin_particle_obj_emitter_data) plugin_particle_obj_emitter_data_list_t;
typedef TAILQ_HEAD(plugin_particle_obj_particle_list, plugin_particle_obj_particle) plugin_particle_obj_particle_list_t;
typedef TAILQ_HEAD(plugin_particle_obj_mod_data_list, plugin_particle_obj_mod_data) plugin_particle_obj_mod_data_list_t;
typedef TAILQ_HEAD(plugin_particle_obj_plugin_list, plugin_particle_obj_plugin) plugin_particle_obj_plugin_list_t;
typedef TAILQ_HEAD(plugin_particle_obj_plugin_data_list, plugin_particle_obj_plugin_data) plugin_particle_obj_plugin_data_list_t;
typedef TAILQ_HEAD(plugin_particle_obj_emitter_binding_list, plugin_particle_obj_emitter_binding) plugin_particle_obj_emitter_binding_list_t;

struct plugin_particle_module {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    int m_debug;

    ui_data_mgr_t m_data_mgr;
    ui_cache_manager_t m_cache_mgr;
    ui_runtime_module_t m_runtime;

    LPDRMETA m_meta_particle_emitter;
    LPDRMETA m_meta_particle_mod;
    LPDRMETA m_meta_particle_mod_data;
    LPDRMETA m_meta_particle_point;

    uint32_t m_free_particle_count;
    uint32_t m_active_particle_count;
    plugin_particle_obj_particle_list_t m_free_particles;
    plugin_particle_obj_mod_data_list_t m_free_mod_datas;
    uint32_t m_free_emitter_count;
    uint32_t m_active_emitter_count;
    plugin_particle_obj_emitter_list_t m_free_emitters;
    plugin_particle_obj_plugin_list_t m_free_plugins;
    plugin_particle_obj_emitter_data_list_t m_free_emitter_datas;
    plugin_particle_obj_emitter_mod_list_t m_free_emitter_mods;
    plugin_particle_obj_emitter_binding_list_t m_free_emitter_bindings;

    struct mem_buffer m_dump_buffer;
};

#ifdef __cplusplus
}
#endif

#endif
