#ifndef PLUGIN_SCROLLMAP_MODULE_I_H
#define PLUGIN_SCROLLMAP_MODULE_I_H
#include "cpe/pal/pal_queue.h"
#include "cpe/utils/buffer.h"
#include "gd/app/app_types.h"
#include "plugin/scrollmap/plugin_scrollmap_module.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef TAILQ_HEAD(plugin_scrollmap_data_tile_list, plugin_scrollmap_data_tile) plugin_scrollmap_data_tile_list_t;
typedef TAILQ_HEAD(plugin_scrollmap_data_layer_list, plugin_scrollmap_data_layer) plugin_scrollmap_data_layer_list_t;
typedef TAILQ_HEAD(plugin_scrollmap_data_block_list, plugin_scrollmap_data_block) plugin_scrollmap_data_block_list_t;
typedef TAILQ_HEAD(plugin_scrollmap_data_script_list, plugin_scrollmap_data_script) plugin_scrollmap_data_script_list_t;

typedef TAILQ_HEAD(plugin_scrollmap_obj_type_map_list, plugin_scrollmap_obj_type_map) plugin_scrollmap_obj_type_map_list_t;

struct plugin_scrollmap_module {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    ui_data_mgr_t m_data_mgr;
    ui_runtime_module_t m_runtime;
    plugin_moving_module_t m_moving_module;
    error_monitor_t m_em;

    LPDRMETA m_meta_script;
    LPDRMETA m_meta_tile;
    LPDRMETA m_meta_layer;
    LPDRMETA m_meta_block;

    plugin_scrollmap_data_tile_list_t m_free_data_tiles;
    plugin_scrollmap_data_layer_list_t m_free_data_layers;
    plugin_scrollmap_data_block_list_t m_free_data_blocks;
    plugin_scrollmap_data_script_list_t m_free_data_scripts;

    int m_debug;
};

#ifdef __cplusplus
}
#endif

#endif
