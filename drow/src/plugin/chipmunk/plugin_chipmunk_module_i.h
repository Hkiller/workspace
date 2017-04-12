#ifndef PLUGIN_CHIPMUNK_MODULE_I_H
#define PLUGIN_CHIPMUNK_MODULE_I_H
#include "cpe/pal/pal_queue.h"
#include "cpe/utils/memory.h"
#include "cpe/utils/error.h"
#include "cpe/utils/buffer.h"
#include "gd/app/app_context.h"
#include "plugin/chipmunk/plugin_chipmunk_module.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef TAILQ_HEAD(plugin_chipmunk_data_body_list, plugin_chipmunk_data_body) plugin_chipmunk_data_body_list_t;
typedef TAILQ_HEAD(plugin_chipmunk_data_fixture_list, plugin_chipmunk_data_fixture) plugin_chipmunk_data_fixture_list_t;
typedef TAILQ_HEAD(plugin_chipmunk_data_polygon_node_list, plugin_chipmunk_data_polygon_node) plugin_chipmunk_data_polygon_node_list_t;
typedef TAILQ_HEAD(plugin_chipmunk_data_constraint_list, plugin_chipmunk_data_constraint) plugin_chipmunk_data_constraint_list_t;
typedef TAILQ_HEAD(plugin_chipmunk_env_updator_list, plugin_chipmunk_env_updator) plugin_chipmunk_env_updator_list_t;

struct plugin_chipmunk_module {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    uint8_t m_debug;
    ui_data_mgr_t m_data_mgr;
    ui_runtime_module_t m_runtime;

    LPDRMETA m_meta_data_scene;
    LPDRMETA m_meta_data_body;
    LPDRMETA m_meta_data_fixture;
    LPDRMETA m_meta_data_polygon_node;
    LPDRMETA m_meta_data_constraint;

    plugin_chipmunk_env_updator_list_t m_free_updators;
    
    struct mem_buffer m_dump_buffer;
};

#ifdef __cplusplus
}
#endif

#endif 
