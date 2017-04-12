#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/tailq_sort.h"
#include "plugin_scrollmap_data_block_i.h"

plugin_scrollmap_data_block_t
plugin_scrollmap_data_block_create(plugin_scrollmap_data_layer_t layer) {
    plugin_scrollmap_module_t module = layer->m_scene->m_module;
    plugin_scrollmap_data_block_t block;

    block = TAILQ_FIRST(&module->m_free_data_blocks);
    if (block) {
        TAILQ_REMOVE(&module->m_free_data_blocks, block, m_next);
    }
    else {
        block = mem_alloc(module->m_alloc, sizeof(struct plugin_scrollmap_data_block));
        if (block == NULL) {
            CPE_ERROR(module->m_em, "plugin_scrollmap_data_block_create: alloc fail!");
            return NULL;
        }
    }

    bzero(&block->m_data, sizeof(block->m_data));
    block->m_layer = layer;

    layer->m_block_count++;
    TAILQ_INSERT_TAIL(&layer->m_blocks, block, m_next);

    return block;
}

void plugin_scrollmap_data_block_free(plugin_scrollmap_data_block_t block) {
    plugin_scrollmap_module_t module = block->m_layer->m_scene->m_module;

    assert(block->m_layer->m_block_count > 0);
    block->m_layer->m_block_count--;
    TAILQ_REMOVE(&block->m_layer->m_blocks, block, m_next);

    block->m_layer = (void*)module;
    TAILQ_INSERT_TAIL(&module->m_free_data_blocks, block, m_next);
}

void plugin_scrollmap_data_block_real_free(plugin_scrollmap_data_block_t block) {
    plugin_scrollmap_module_t module = (void*)block->m_layer;
    TAILQ_REMOVE(&module->m_free_data_blocks, block, m_next);
    mem_free(module->m_alloc, block);
}

SCROLLMAP_BLOCK * plugin_scrollmap_data_block_data(plugin_scrollmap_data_block_t block) {
    return &block->m_data;
}

static int plugin_scrollmap_manip_block_cmp(plugin_scrollmap_data_block_t l, plugin_scrollmap_data_block_t r, void * p) {
    return (int)(l->m_data.pos.y - r->m_data.pos.y);
}

void plugin_scrollmap_data_layer_sort_blocks(plugin_scrollmap_data_layer_t layer) {
    TAILQ_SORT(
        &layer->m_blocks,
        plugin_scrollmap_data_block,
        plugin_scrollmap_data_block_list,
        m_next,
        plugin_scrollmap_manip_block_cmp, NULL);
}

static plugin_scrollmap_data_block_t plugin_scrollmap_data_block_next(struct plugin_scrollmap_data_block_it * it) {
    plugin_scrollmap_data_block_t * data = (plugin_scrollmap_data_block_t *)(it->m_data);
    plugin_scrollmap_data_block_t r;

    if (*data == NULL) return NULL;

    r = *data;

    *data = TAILQ_NEXT(r, m_next);

    return r;
}

void plugin_scrollmap_data_layer_blocks(plugin_scrollmap_data_layer_t layer, plugin_scrollmap_data_block_it_t it) {
    *(plugin_scrollmap_data_block_t *)(it->m_data) = TAILQ_FIRST(&layer->m_blocks);
    it->next = plugin_scrollmap_data_block_next;
}
