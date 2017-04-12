#ifndef PLUGIN_SCROLLMAP_TILE_I_H
#define PLUGIN_SCROLLMAP_TILE_I_H
#include "render/utils/ui_transform.h"
#include "render/utils/ui_rect.h"
#include "plugin/scrollmap/plugin_scrollmap_data_scene.h"
#include "plugin_scrollmap_env_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_scrollmap_tile {
    plugin_scrollmap_env_t m_env;
    union {
        struct cpe_hash_entry m_hh;
        TAILQ_ENTRY(plugin_scrollmap_tile) m_next;
    };
    SCROLLMAP_TILE const * m_data;
    union {
        struct {
            ui_rect m_uv;
        } m_module;
    };
    ui_transform m_transform;
};

plugin_scrollmap_tile_t plugin_scrollmap_tile_create(plugin_scrollmap_env_t context, SCROLLMAP_TILE const * tile);
void plugin_scrollmap_tile_free(plugin_scrollmap_tile_t tile);

plugin_scrollmap_tile_t plugin_scrollmap_tile_find(plugin_scrollmap_env_t context, SCROLLMAP_TILE const * tile);
    
void plugin_scrollmap_tile_real_free(plugin_scrollmap_tile_t tile);

void plugin_scrollmap_tile_free_all(plugin_scrollmap_env_t env);

uint32_t plugin_scrollmap_tile_hash(const plugin_scrollmap_tile_t tile);
int plugin_scrollmap_tile_eq(const plugin_scrollmap_tile_t l, const plugin_scrollmap_tile_t r);

#ifdef __cplusplus
}
#endif

#endif
