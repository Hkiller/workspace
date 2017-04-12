#include "plugin_scrollmap_block_i.h"
#include "plugin_scrollmap_layer_i.h"
#include "plugin_scrollmap_range_i.h"

plugin_scrollmap_block_t
plugin_scrollmap_block_create(
    plugin_scrollmap_layer_t layer, plugin_scrollmap_range_t range, SCROLLMAP_TILE const * tile_data, SCROLLMAP_PAIR const * pos)
{
    plugin_scrollmap_env_t env = layer->m_env;
    plugin_scrollmap_module_t module = env->m_module;
    plugin_scrollmap_block_t block;
    plugin_scrollmap_block_t insert_after;
    plugin_scrollmap_tile_t tile;

    tile = plugin_scrollmap_tile_find(env, tile_data);
    if (tile == NULL) {
        tile = plugin_scrollmap_tile_create(env, tile_data);
        if (tile == NULL) {
            CPE_ERROR(module->m_em, "scrollmap_tile_create: create tile fail!");
            return NULL;
        }
    }

    block = TAILQ_FIRST(&env->m_free_blocks);
    if (block) {
        TAILQ_REMOVE(&env->m_free_blocks, block, m_next_for_layer);
    }
    else {
        block = mem_alloc(module->m_alloc, sizeof(struct plugin_scrollmap_block));
        if (block == NULL) return NULL;
    }

    block->m_tile = tile;
    block->m_pos = *pos;

    block->m_range = range;
    TAILQ_INSERT_TAIL(&range->m_blocks, block, m_next_for_range);

    block->m_layer = layer;
    /*connect to layer and source*/
    for(insert_after = TAILQ_LAST(&layer->m_blocks, plugin_scrollmap_block_list);
        insert_after != TAILQ_END(&layer->m_blocks);
        insert_after = TAILQ_PREV(insert_after, plugin_scrollmap_block_list, m_next_for_layer)
        )
    {
        if (insert_after->m_pos.y >= block->m_pos.y) {
            TAILQ_INSERT_AFTER(&layer->m_blocks, insert_after, block, m_next_for_layer);
            break;
        }
    }

    if (insert_after == TAILQ_END(&layer->m_blocks)) {
        TAILQ_INSERT_HEAD(&layer->m_blocks, block, m_next_for_layer);
    }

    return block;
}

SCROLLMAP_TILE const * plugin_scrollmap_block_tile(plugin_scrollmap_block_t block) {
	return block->m_tile->m_data;
}

SCROLLMAP_PAIR const * plugin_scrollmap_block_pos(plugin_scrollmap_block_t block) {
	return &block->m_pos;
}

void plugin_scrollmap_block_free(plugin_scrollmap_block_t block) {
    plugin_scrollmap_layer_t layer = block->m_layer;
    plugin_scrollmap_range_t range = block->m_range;

    TAILQ_REMOVE(&layer->m_blocks, block, m_next_for_layer);
    TAILQ_REMOVE(&range->m_blocks, block, m_next_for_range);

    block->m_layer = (void*)layer->m_env;
    TAILQ_INSERT_HEAD(&layer->m_env->m_free_blocks, block, m_next_for_layer);
}

void plugin_scrollmap_block_real_free(plugin_scrollmap_block_t block) {
    plugin_scrollmap_env_t env = (void*)block->m_layer;

    TAILQ_REMOVE(&env->m_free_blocks, block, m_next_for_layer);

    mem_free(env->m_module->m_alloc, block);
}

