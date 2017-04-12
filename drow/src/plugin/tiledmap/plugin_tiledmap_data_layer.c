#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_strings.h"
#include "render/utils/ui_rect.h"
#include "plugin_tiledmap_data_layer_i.h"
#include "plugin_tiledmap_data_scene_i.h"
#include "plugin_tiledmap_data_tile_i.h"

plugin_tiledmap_data_layer_t
plugin_tiledmap_data_layer_create(plugin_tiledmap_data_scene_t data_scene) {
    plugin_tiledmap_module_t module = data_scene->m_module;
    plugin_tiledmap_data_layer_t data_layer;

    data_layer = (plugin_tiledmap_data_layer_t)mem_alloc(module->m_alloc, sizeof(struct plugin_tiledmap_data_layer));
    if(data_layer == NULL) {
        CPE_ERROR(module->m_em, "create layer proto: alloc fail!");
        return NULL;
    }

    bzero(data_layer, sizeof(*data_layer));
    data_layer->m_scene = data_scene;
    TAILQ_INIT(&data_layer->m_tile_list);
        
    data_scene->m_layer_count++;
    TAILQ_INSERT_TAIL(&data_scene->m_layer_list, data_layer, m_next_for_scene);

    return data_layer;
}

void plugin_tiledmap_data_layer_free(plugin_tiledmap_data_layer_t data_layer) {
    plugin_tiledmap_data_scene_t data_scene = data_layer->m_scene;
    plugin_tiledmap_module_t module = data_scene->m_module;

    while(!TAILQ_EMPTY(&data_layer->m_tile_list)) {
        plugin_tiledmap_data_tile_free(TAILQ_FIRST(&data_layer->m_tile_list));
    }

    data_scene->m_layer_count--;
    TAILQ_REMOVE(&data_scene->m_layer_list, data_layer, m_next_for_scene);

    mem_free(module->m_alloc, data_layer);
}

const char * plugin_tiledmap_data_layer_name(plugin_tiledmap_data_layer_t layer) {
    return layer->m_data.name;
}

plugin_tiledmap_data_layer_t
plugin_tiledmap_data_layer_find_by_name(plugin_tiledmap_data_scene_t scene, const char * name) {
    plugin_tiledmap_data_layer_t r;

    TAILQ_FOREACH(r, &scene->m_layer_list, m_next_for_scene) {
        if (strcmp(r->m_data.name, name) == 0) return r;
    }

    return NULL;
}

plugin_tiledmap_data_scene_t plugin_tiledmap_data_layer_scene(plugin_tiledmap_data_layer_t layer) {
    return layer->m_scene;
}

TILEDMAP_LAYER * plugin_tiledmap_data_layer_data(plugin_tiledmap_data_layer_t data_layer) {
    return &data_layer->m_data;
}

uint32_t plugin_tiledmap_data_layer_tile_count(plugin_tiledmap_data_layer_t layer) {
    return layer->m_tile_count;
}

void plugin_tiledmap_data_layer_config_rect(plugin_tiledmap_data_layer_t layer, ui_rect_t rect) {
    rect->lt.x = (float)layer->m_data.cell_col_begin * layer->m_data.cell_w;
    rect->lt.y = (float)layer->m_data.cell_row_begin * layer->m_data.cell_h;
    rect->rb.x = (float)layer->m_data.cell_col_end * layer->m_data.cell_w;
    rect->rb.y = (float)layer->m_data.cell_row_end * layer->m_data.cell_h;
}

int plugin_tiledmap_data_layer_rect(plugin_tiledmap_data_layer_t layer, ui_rect_t rect) {
    uint8_t have_data = 0;
    plugin_tiledmap_data_tile_t tile;
    ui_data_src_t src_cache = NULL;

    TAILQ_FOREACH(tile, &layer->m_tile_list, m_next_for_layer) {
        ui_rect tile_rect;
        if (plugin_tiledmap_data_tile_rect(tile, &tile_rect, &src_cache) != 0) return -1;

        if (have_data) {
            if (tile_rect.lt.x < rect->lt.x) rect->lt.x = tile_rect.lt.x;
            if (tile_rect.lt.y < rect->lt.y) rect->lt.y = tile_rect.lt.y;
            if (tile_rect.rb.x > rect->rb.x) rect->rb.x = tile_rect.rb.x;
            if (tile_rect.rb.y > rect->rb.y) rect->rb.y = tile_rect.rb.y;
        }
        else {
            have_data = 1;
            *rect = tile_rect;
        }
    }

    if (!have_data) {
        rect->lt.x = 0.0f;
        rect->lt.y = 0.0f;
        rect->rb.x = 0.0f;
        rect->rb.y = 0.0f;
    }

    return 0;
}
    
static plugin_tiledmap_data_tile_t plugin_tiledmap_data_layer_tile_next(struct plugin_tiledmap_data_tile_it * it) {
    plugin_tiledmap_data_tile_t * data = (plugin_tiledmap_data_tile_t *)(it->m_data);
    plugin_tiledmap_data_tile_t r;

    if (*data == NULL) return NULL;

    r = *data;

    *data = TAILQ_NEXT(r, m_next_for_layer);

    return r;
}

void plugin_tiledmap_data_layer_tiles(plugin_tiledmap_data_tile_it_t tile_it, plugin_tiledmap_data_layer_t layer) {
    *(plugin_tiledmap_data_tile_t *)(tile_it->m_data) = TAILQ_FIRST(&layer->m_tile_list);
    tile_it->next = plugin_tiledmap_data_layer_tile_next;
}
