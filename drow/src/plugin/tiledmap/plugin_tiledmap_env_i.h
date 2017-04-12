#ifndef PLUGIN_TILEDMAP_ENV_I_H
#define PLUGIN_TILEDMAP_ENV_I_H
#include "cpe/utils/hash.h"
#include "plugin/tiledmap/plugin_tiledmap_env.h"
#include "plugin_tiledmap_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef TAILQ_HEAD(plugin_tiledmap_tile_list, plugin_tiledmap_tile) plugin_tiledmap_tile_list_t;
    
struct plugin_tiledmap_env {
    plugin_tiledmap_module_t m_module;
    struct cpe_hash_table m_layers;
    plugin_tiledmap_tile_list_t m_free_tiles;
    void * m_extern_obj_create_ctx;
    plugin_tiledmap_env_extern_obj_create_fun_t m_extern_obj_create_fun;
};

#ifdef __cplusplus
}
#endif

#endif
