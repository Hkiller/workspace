#ifndef PLUGIN_TILEDMAP_MODULE_I_H
#define PLUGIN_TILEDMAP_MODULE_I_H
#include "cpe/pal/pal_queue.h"
#include "cpe/utils/memory.h"
#include "cpe/utils/error.h"
#include "cpe/utils/buffer.h"
#include "gd/app/app_context.h"
#include "render/cache/ui_cache_manager.h"
#include "plugin/tiledmap/plugin_tiledmap_module.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef TAILQ_HEAD(plugin_tiledmap_data_layer_list, plugin_tiledmap_data_layer) plugin_tiledmap_data_layer_list_t;
typedef TAILQ_HEAD(plugin_tiledmap_data_tile_list, plugin_tiledmap_data_tile) plugin_tiledmap_data_tile_list_t;
    
struct plugin_tiledmap_module {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    uint8_t m_debug;
    ui_data_mgr_t m_data_mgr;
    ui_runtime_module_t m_runtime;

    LPDRMETA m_meta_data_scene;
    LPDRMETA m_meta_data_layer;
    LPDRMETA m_meta_data_tile;

    struct mem_buffer m_dump_buffer;
};

#ifdef __cplusplus
}
#endif

#endif 
