#ifndef PLUGIN_TILEDMAP_LAYER_I_H
#define PLUGIN_TILEDMAP_LAYER_I_H
#include "render/utils/ui_transform.h"
#include "plugin/tiledmap/plugin_tiledmap_layer.h"
#include "plugin_tiledmap_env_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_tiledmap_layer {
    plugin_tiledmap_env_t m_env;
    char m_name[64];
    struct cpe_hash_entry m_hh_for_env;
    struct ui_transform m_trans;
    float m_alpha;
    plugin_tiledmap_tile_list_t m_tiles;
};

void plugin_tiledmap_layer_free_all(plugin_tiledmap_env_t env);

uint32_t plugin_tiledmap_hash(plugin_tiledmap_layer_t layer);
uint32_t plugin_tiledmap_eq(plugin_tiledmap_layer_t l, plugin_tiledmap_layer_t r);

#ifdef __cplusplus
}
#endif

#endif
