#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/pal/pal_stdio.h"
#include "render/model/ui_data_src.h"
#include "render/model/ui_data_sprite.h"
#include "render/model/ui_data_mgr.h"
#include "render/model/ui_data_module.h"
#include "render/cache/ui_cache_manager.h"
#include "render/cache/ui_cache_texture.h"
#include "plugin/tiledmap/plugin_tiledmap_data_tile.h"
#include "plugin_tiledmap_tile_i.h"

plugin_tiledmap_tile_t
plugin_tiledmap_tile_create(
    plugin_tiledmap_layer_t layer, plugin_tiledmap_data_tile_t data_tile, ui_vector_2_t pos, ui_data_src_t using_src)
{
    plugin_tiledmap_module_t module = layer->m_env->m_module;
    plugin_tiledmap_tile_t tile;
    TILEDMAP_TILE const * tile_data = plugin_tiledmap_data_tile_data(data_tile);
    
    tile = TAILQ_FIRST(&layer->m_env->m_free_tiles);
    if (tile == NULL) {
        tile = mem_alloc(module->m_alloc, sizeof(struct plugin_tiledmap_tile));
        if (tile == NULL) {
            CPE_ERROR(module->m_em, "plugin_tiledmap_tile_create: alloc fail!");
            return NULL;
        }
    }
    else {
        TAILQ_REMOVE(&layer->m_env->m_free_tiles, tile, m_next);
    }

    bzero(tile, sizeof(*tile));
    
    tile->m_layer = layer;
    tile->m_data_tile = data_tile;
    tile->m_pos = *pos;

    switch(tile_data->ref_type) {
    case tiledmap_tile_ref_type_img: {
        ui_data_module_t using_module = ui_data_src_product(using_src);
        ui_data_img_block_t using_img_block;
        ui_cache_res_t using_texture;

        using_img_block = ui_data_img_block_find_by_id(using_module, tile_data->ref_data.img.img_block_id);
        if (using_img_block == NULL) {
            CPE_ERROR(
                module->m_em, "plugin_tiledmap_tile_create: using img block "FMT_UINT32_T" not exist in module "FMT_UINT32_T"!",
                tile_data->ref_data.img.img_block_id, ui_data_src_id(using_src));
            tile->m_layer = (void*)layer->m_env;
            TAILQ_INSERT_TAIL(&layer->m_env->m_free_tiles, tile, m_next);
            return NULL;
        }

        using_texture = ui_data_img_block_using_texture(using_img_block);
        if (using_texture == NULL) {
            CPE_ERROR(
                module->m_em, "plugin_tiledmap_tile_create: module "FMT_UINT32_T" using img %s not in cache!",
                ui_data_src_id(using_src), ui_data_img_block_using_texture_path(using_img_block));
            tile->m_layer = (void*)layer->m_env;
            TAILQ_INSERT_TAIL(&layer->m_env->m_free_tiles, tile, m_next);
            return NULL;
        }

        tile->m_using_texture = using_texture;
        tile->m_using_product = using_img_block;
        break;
    }
    case tiledmap_tile_ref_type_frame: {
        ui_data_sprite_t using_sprite = ui_data_src_product(using_src);
        ui_data_frame_t using_frame;

        using_frame = ui_data_frame_find_by_id(using_sprite, tile_data->ref_data.frame.frame_id);
        if (using_frame == NULL) {
            CPE_ERROR(
                module->m_em, "plugin_tiledmap_tile_create: using frame "FMT_UINT32_T" not exist in sprite "FMT_UINT32_T"!",
                tile_data->ref_data.img.img_block_id, ui_data_src_id(using_src));
            tile->m_layer = (void*)layer->m_env;
            TAILQ_INSERT_TAIL(&layer->m_env->m_free_tiles, tile, m_next);
            return NULL;
        }

        tile->m_using_texture = NULL;
        tile->m_using_product = using_frame;
        break;
    }
    case tiledmap_tile_ref_type_tag: {
        tile->m_using_texture = NULL;
        tile->m_using_product = NULL;
        break;
    }
    default:
        CPE_ERROR(module->m_em, "plugin_tiledmap_tile_create: not support ref type %d!", tile_data->ref_type);
        tile->m_layer = (void*)layer->m_env;
        TAILQ_INSERT_TAIL(&layer->m_env->m_free_tiles, tile, m_next);
        return NULL;
    }

    TAILQ_INSERT_TAIL(&layer->m_tiles, tile, m_next);

    return tile;
}

void plugin_tiledmap_tile_free(plugin_tiledmap_tile_t tile) {
    plugin_tiledmap_layer_t layer = tile->m_layer;

    TAILQ_REMOVE(&layer->m_tiles, tile, m_next);

    tile->m_layer = (void*)layer->m_env;
    TAILQ_INSERT_TAIL(&layer->m_env->m_free_tiles, tile, m_next);
}

void plugin_tiledmap_tile_real_free(plugin_tiledmap_tile_t tile) {
    plugin_tiledmap_env_t env = (void*)tile->m_layer;

    TAILQ_REMOVE(&env->m_free_tiles, tile, m_next);
    mem_free(env->m_module->m_alloc, tile);
}

plugin_tiledmap_layer_t plugin_tiledmap_tile_layer(plugin_tiledmap_tile_t tile) {
    return tile->m_layer;
}

plugin_tiledmap_data_tile_t plugin_tiledmap_tile_data(plugin_tiledmap_tile_t tile) {
    return tile->m_data_tile;
}

ui_vector_2_t plugin_tiledmap_tile_pos(plugin_tiledmap_tile_t tile) {
    return &tile->m_pos;
}

ui_cache_res_t plugin_tiledmap_tile_using_texture(plugin_tiledmap_tile_t tile) {
    return tile->m_using_texture;
}

ui_data_img_block_t plugin_tiledmap_tile_using_img_block(plugin_tiledmap_tile_t tile) {
    TILEDMAP_TILE const * tile_data = plugin_tiledmap_data_tile_data(tile->m_data_tile);
    return tile_data->ref_type == tiledmap_tile_ref_type_img ? tile->m_using_product : NULL;
}

ui_data_frame_t plugin_tiledmap_tile_useing_frame(plugin_tiledmap_tile_t tile) {
    TILEDMAP_TILE const * tile_data = plugin_tiledmap_data_tile_data(tile->m_data_tile);
    return tile_data->ref_type == tiledmap_tile_ref_type_frame ? tile->m_using_product : NULL;
}
