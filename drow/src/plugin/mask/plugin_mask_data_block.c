#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/bitarry.h"
#include "render/model/ui_data_src.h"
#include "plugin_mask_data_block_i.h"

plugin_mask_data_block_t plugin_mask_data_block_create(
    plugin_mask_data_t data, uint32_t name,
    int32_t x, int32_t y, uint32_t width, uint32_t heigh,
    uint32_t buf_x, uint32_t buf_y, uint32_t buf_width, uint32_t buf_height)
{
    plugin_mask_module_t module = data->m_module;
    plugin_mask_data_block_t block;

    block = TAILQ_FIRST(&module->m_free_data_blocks);
    if (block) {
        TAILQ_REMOVE(&module->m_free_data_blocks, block, m_next);
    }
    else {
        block = mem_alloc(module->m_alloc, sizeof(struct plugin_mask_data_block));
        if (block == NULL) {
            CPE_ERROR(module->m_em, "plugin_mask_data_block_create: alloc fail!");
            return NULL;
        }
    }

    bzero(&block->m_data, sizeof(block->m_data));
    block->m_data = data;
    block->m_name = name;
    block->m_x = x;
    block->m_y = y;
    block->m_width = width;
    block->m_height = heigh;
    block->m_buf_x = buf_x;
    block->m_buf_y = buf_y;
    block->m_buf_width = buf_width;
    block->m_buf_height = buf_height;
    block->m_buf = NULL;
    block->m_buf_size = plugin_mask_data_block_buf_line_size(block) * buf_height;

    data->m_block_count++;
    TAILQ_INSERT_TAIL(&data->m_blocks, block, m_next);

    return block;
}

void plugin_mask_data_block_free(plugin_mask_data_block_t block) {
    plugin_mask_module_t module = block->m_data->m_module;

    if (block->m_buf) {
        mem_free(module->m_alloc, block->m_buf);
        block->m_buf = NULL;
    }
    
    assert(block->m_data->m_block_count > 0);
    block->m_data->m_block_count--;
    TAILQ_REMOVE(&block->m_data->m_blocks, block, m_next);

    block->m_data = (void*)module;
    TAILQ_INSERT_TAIL(&module->m_free_data_blocks, block, m_next);
}

void plugin_mask_data_block_real_free(plugin_mask_data_block_t block) {
    plugin_mask_module_t module = (void*)block->m_data;
    TAILQ_REMOVE(&module->m_free_data_blocks, block, m_next);
    mem_free(module->m_alloc, block);
}

plugin_mask_data_block_t
plugin_mask_data_block_find(plugin_mask_data_t data, const char * name) {
    plugin_mask_data_block_t block;

    TAILQ_FOREACH(block, &data->m_blocks, m_next) {
        if (strcmp(plugin_mask_data_block_name(block), name) == 0) return block;
    }

    return NULL;
}

plugin_mask_data_t plugin_mask_data_block_data(plugin_mask_data_block_t block) {
    return block->m_data;
}

const char * plugin_mask_data_block_name(plugin_mask_data_block_t block) {
    return ui_data_src_msg(block->m_data->m_src, block->m_name);
}

int32_t plugin_mask_data_block_x(plugin_mask_data_block_t block) {
    return block->m_x;
}

int32_t plugin_mask_data_block_y(plugin_mask_data_block_t block) {
    return block->m_y;
}

uint32_t plugin_mask_data_block_width(plugin_mask_data_block_t block) {
    return block->m_width;
}

uint32_t plugin_mask_data_block_height(plugin_mask_data_block_t block) {
    return block->m_height;
}

uint32_t plugin_mask_data_block_buf_x(plugin_mask_data_block_t block) {
    return block->m_buf_x;
}

uint32_t plugin_mask_data_block_buf_y(plugin_mask_data_block_t block) {
    return block->m_buf_y;
}

uint32_t plugin_mask_data_block_buf_width(plugin_mask_data_block_t block) {
    return block->m_buf_width;
}

uint32_t plugin_mask_data_block_buf_height(plugin_mask_data_block_t block) {
    return block->m_buf_height;
}

void * plugin_mask_data_block_buf(plugin_mask_data_block_t block) {
    return block->m_buf;
}

void * plugin_mask_data_block_check_create_buf(plugin_mask_data_block_t block) {
    if (block->m_buf == NULL) {
        block->m_buf = mem_alloc(block->m_data->m_module->m_alloc, block->m_buf_size);
    }
    return block->m_buf;
}

uint32_t plugin_mask_data_block_buf_line_size(plugin_mask_data_block_t block) {
    switch(block->m_data->m_format) {
    case plugin_mask_data_format_bit:
        return cpe_ba_bytes_from_bits_m(block->m_width);
    case plugin_mask_data_format_1:
        return block->m_width;
    case plugin_mask_data_format_2:
        return block->m_width * 2;
    case plugin_mask_data_format_4:
        return block->m_width * 4;
    default:
        return 0;
    }
}

size_t plugin_mask_data_block_buf_size(plugin_mask_data_block_t block) {
    return block->m_buf_size;
}

static plugin_mask_data_block_t plugin_mask_data_block_next(struct plugin_mask_data_block_it * it) {
    plugin_mask_data_block_t * data = (plugin_mask_data_block_t *)(it->m_data);
    plugin_mask_data_block_t r;

    if (*data == NULL) return NULL;

    r = *data;

    *data = TAILQ_NEXT(r, m_next);

    return r;
}

void plugin_mask_data_blocks(plugin_mask_data_t data, plugin_mask_data_block_it_t it) {
    *(plugin_mask_data_block_t *)(it->m_data) = TAILQ_FIRST(&data->m_blocks);
    it->next = plugin_mask_data_block_next;
}
