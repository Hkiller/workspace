#include <assert.h>
#include "cpe/utils/string_utils.h"
#include "plugin_layout_layout_rich_block_i.h"
#include "plugin_layout_layout_i.h"
#include "plugin_layout_render_i.h"

plugin_layout_layout_rich_block_t
plugin_layout_layout_rich_block_create(plugin_layout_layout_rich_t rich) {
    plugin_layout_module_t module = plugin_layout_layout_from_data(rich)->m_render->m_module;
    plugin_layout_layout_rich_block_t block;

    block = TAILQ_FIRST(&module->m_free_layout_rich_blocks);
    if (block) {
        TAILQ_REMOVE(&module->m_free_layout_rich_blocks, block, m_next);
    }
    else {
        block = mem_alloc(module->m_alloc, sizeof(struct plugin_layout_layout_rich_block));
        if (block == NULL) {
            CPE_ERROR(module->m_em, "plugin_layout_layout_rich_block_create: alloc fail!");
            return NULL;
        }
    }

    block->m_rich = rich;
    block->m_font_id = &rich->m_default_font_id;
    block->m_font_draw = &rich->m_default_font_draw;
    block->m_text_begin = NULL;
    block->m_text_end = NULL;
    block->m_text_own = 0;

    TAILQ_INSERT_TAIL(&rich->m_blocks, block, m_next);
    
    return block;
}

void plugin_layout_layout_rich_block_free(plugin_layout_layout_rich_block_t block) {
    plugin_layout_module_t module = plugin_layout_layout_from_data(block->m_rich)->m_render->m_module;

    TAILQ_REMOVE(&block->m_rich->m_blocks, block, m_next);

    if (block->m_text_own) {
        assert(block->m_text_begin);
        mem_free(module->m_alloc, (void*)block->m_text_begin);
    }

    block->m_rich = (plugin_layout_layout_rich_t)module;
    TAILQ_INSERT_TAIL(&module->m_free_layout_rich_blocks, block, m_next);
}

void plugin_layout_layout_rich_block_real_free(plugin_layout_layout_rich_block_t block) {
    plugin_layout_module_t module = (plugin_layout_module_t)block->m_rich;
    TAILQ_REMOVE(&module->m_free_layout_rich_blocks, block, m_next);
    mem_free(module->m_alloc, block);
}

void plugin_layout_layout_rich_block_set_color(plugin_layout_layout_rich_block_t block, ui_color_t color) {
    if (block->m_font_draw != &block->m_buf_font_draw) {
        block->m_buf_font_draw = *block->m_font_draw;
        block->m_font_draw = &block->m_buf_font_draw;
    }

    block->m_font_draw->color = *color;
}

void plugin_layout_layout_rich_block_set_size(plugin_layout_layout_rich_block_t block, uint8_t size) {
    if (block->m_font_id != &block->m_buf_font_id) {
        block->m_buf_font_id = *block->m_font_id;
        block->m_font_id = &block->m_buf_font_id;
    }

    block->m_font_id->size = size;
}

void plugin_layout_layout_rich_block_set_adj_size(plugin_layout_layout_rich_block_t block, int8_t size_adj) {
    if (block->m_font_id != &block->m_buf_font_id) {
        block->m_buf_font_id = *block->m_font_id;
        block->m_font_id = &block->m_buf_font_id;
    }

    if (size_adj < 0) {
        size_adj = - size_adj;
        if (block->m_font_id->size > (uint8_t)size_adj) {
            block->m_font_id->size -= (uint8_t)size_adj;
        }
        else {
            block->m_font_id->size = 1;
        }
    }
    else {
        block->m_font_id->size += (uint8_t)size_adj;
    }
}

void plugin_layout_layout_rich_block_set_font_id(plugin_layout_layout_rich_block_t block, plugin_layout_font_id_t font_id) {
    block->m_buf_font_id = *font_id;
    block->m_font_id = &block->m_buf_font_id;
}

void plugin_layout_layout_rich_block_set_font_draw(plugin_layout_layout_rich_block_t block, plugin_layout_font_draw_t font_draw) {
    block->m_buf_font_draw = *font_draw;
    block->m_font_draw = &block->m_buf_font_draw;
}

int plugin_layout_layout_rich_block_set_context_range(
    plugin_layout_layout_rich_block_t block, const char * begin, const char *  end, uint8_t own)
{
    plugin_layout_module_t module = plugin_layout_layout_from_data(block->m_rich)->m_render->m_module;
    
    if (block->m_text_own) {
        assert(block->m_text_begin);
        mem_free(module->m_alloc, (void*)block->m_text_begin);
    }

    if (begin == end) {
        block->m_text_begin = NULL;
        block->m_text_end = NULL;
        block->m_text_own = 0;
        return 0;
    }

    if (own) {
        block->m_text_begin = cpe_str_mem_dup_range(module->m_alloc, begin, end);
        if (block->m_text_begin == NULL) {
            CPE_ERROR(module->m_em, "plugin_layout_layout_rich_block_set_context_range: dup range fail!");
            block->m_text_own = 0;
            block->m_text_end = NULL;
            return -1;
        }
        else {
            block->m_text_end = block->m_text_begin + (end - begin);
            block->m_text_own = 1;
            return 0;
        }
    }
    else {
        block->m_text_begin = begin;
        block->m_text_end = end;
        block->m_text_own = 0;
        return 0;
    }
}

int plugin_layout_layout_rich_block_set_context(plugin_layout_layout_rich_block_t block, const char * data, uint8_t own) {
    return plugin_layout_layout_rich_block_set_context_range(block, data, data + strlen(data), own);
}
