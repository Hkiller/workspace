#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "render/model/ui_data_src.h"
#include "plugin_scrollmap_data_tile_i.h"

plugin_scrollmap_data_tile_t
plugin_scrollmap_data_tile_create(plugin_scrollmap_data_scene_t scene) {
    plugin_scrollmap_module_t module = scene->m_module;
    plugin_scrollmap_data_tile_t tile;

    tile = TAILQ_FIRST(&module->m_free_data_tiles);
    if (tile) {
        TAILQ_REMOVE(&module->m_free_data_tiles, tile, m_next);
    }
    else {
        tile = mem_alloc(module->m_alloc, sizeof(struct plugin_scrollmap_data_tile));
        if (tile == NULL) {
            CPE_ERROR(module->m_em, "plugin_scrollmap_data_tile_create: alloc fail!");
            return NULL;
        }
    }

    bzero(&tile->m_data, sizeof(tile->m_data));
    tile->m_scene = scene;
    
    scene->m_tile_count++;
    TAILQ_INSERT_TAIL(&scene->m_tiles, tile, m_next);

    return tile;
}

void plugin_scrollmap_data_tile_free(plugin_scrollmap_data_tile_t tile) {
    plugin_scrollmap_module_t module = tile->m_scene->m_module;

    assert(tile->m_scene->m_tile_count > 0);
    tile->m_scene->m_tile_count--;
    TAILQ_REMOVE(&tile->m_scene->m_tiles, tile, m_next);

    tile->m_scene = (void*)module;
    TAILQ_INSERT_TAIL(&module->m_free_data_tiles, tile, m_next);
}

void plugin_scrollmap_data_tile_real_free(plugin_scrollmap_data_tile_t tile) {
    plugin_scrollmap_module_t module = (void*)tile->m_scene;
    TAILQ_REMOVE(&module->m_free_data_tiles, tile, m_next);
    mem_free(module->m_alloc, tile);
}

plugin_scrollmap_data_tile_t
plugin_scrollmap_data_tile_find(plugin_scrollmap_data_scene_t scene, uint16_t tile_id) {
    plugin_scrollmap_data_tile_t tile;

    TAILQ_FOREACH(tile, &scene->m_tiles, m_next) {
        if (tile->m_data.id == tile_id) return tile;
    }

    return NULL;
}

SCROLLMAP_TILE * plugin_scrollmap_data_tile_data(plugin_scrollmap_data_tile_t tile) {
    return &tile->m_data;
}

static plugin_scrollmap_data_tile_t plugin_scrollmap_data_tile_next(struct plugin_scrollmap_data_tile_it * it) {
    plugin_scrollmap_data_tile_t * data = (plugin_scrollmap_data_tile_t *)(it->m_data);
    plugin_scrollmap_data_tile_t r;

    if (*data == NULL) return NULL;

    r = *data;

    *data = TAILQ_NEXT(r, m_next);

    return r;
}

void plugin_scrollmap_data_scene_tiles(plugin_scrollmap_data_scene_t scene, plugin_scrollmap_data_tile_it_t it) {
    *(plugin_scrollmap_data_tile_t *)(it->m_data) = TAILQ_FIRST(&scene->m_tiles);
    it->next = plugin_scrollmap_data_tile_next;
}
