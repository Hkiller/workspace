#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "plugin_pack_block_i.h"
#include "plugin_pack_block_ref_i.h"
#include "plugin_pack_texture_part_i.h"

plugin_pack_block_t
plugin_pack_block_create(
    plugin_pack_texture_t texture, plugin_pack_texture_part_t part,
    cpe_md5_value_t md5, ui_cache_res_t src_texture, ui_cache_pixel_buf_rect_t src_rect)
{
    plugin_pack_module_t module = texture->m_packer->m_module;
    plugin_pack_block_t block;

    block = mem_alloc(module->m_alloc, sizeof(struct plugin_pack_block));
    if (block == NULL) {
        CPE_ERROR(module->m_em, "plugin_pack_block_create: alloc fail!");
        return NULL;
    }

    block->m_texture = texture;
    
    memcpy(&block->m_md5, md5, sizeof(block->m_md5));

    bzero(&block->m_to_rect, sizeof(block->m_to_rect));
    block->m_src_texture = src_texture;
    block->m_src_rect = *src_rect;
    TAILQ_INIT(&block->m_refs);
    
    cpe_hash_entry_init(&block->m_hh);
    if (cpe_hash_table_insert_unique(&texture->m_blocks, block) != 0) {
        CPE_ERROR(module->m_em, "plugin_pack_block_create: block duplicate!");
        mem_free(module->m_alloc, block);
        return NULL;
    }

    block->m_part = part;
    if (part) {
        TAILQ_INSERT_TAIL(&part->m_blocks, block, m_next_for_part);
    }
    
    return block;
}

plugin_pack_block_t plugin_pack_block_find(plugin_pack_texture_t texture, cpe_md5_value_t md5) {
    struct plugin_pack_block key;

    memcpy(&key.m_md5, md5, sizeof(key.m_md5));

    return (plugin_pack_block_t)cpe_hash_table_find(&texture->m_blocks, &key);
}

void plugin_pack_block_free(plugin_pack_block_t block) {
    plugin_pack_texture_t texture = block->m_texture;
    plugin_pack_module_t module = texture->m_packer->m_module;

    while(!TAILQ_EMPTY(&block->m_refs)) {
        plugin_pack_block_ref_free(TAILQ_FIRST(&block->m_refs));
    }

    if (block->m_part) {
        TAILQ_REMOVE(&block->m_part->m_blocks, block, m_next_for_part);
        block->m_part = NULL;
    }
    
    cpe_hash_table_remove_by_ins(&texture->m_blocks, block);
    
    mem_free(module->m_alloc, block);
}

void plugin_pack_block_free_all(plugin_pack_texture_t texture) {
    struct cpe_hash_it block_it;
    plugin_pack_block_t block;

    cpe_hash_it_init(&block_it, &texture->m_blocks);

    block = (plugin_pack_block_t)cpe_hash_it_next(&block_it);
    while (block) {
        plugin_pack_block_t next = (plugin_pack_block_t)cpe_hash_it_next(&block_it);
        plugin_pack_block_free(block);
        block = next;
    }
}

void plugin_pack_block_free_by_src_texture(plugin_pack_texture_t texture, ui_cache_res_t src_texture) {
    struct cpe_hash_it block_it;
    plugin_pack_block_t block;

    cpe_hash_it_init(&block_it, &texture->m_blocks);

    block = (plugin_pack_block_t)cpe_hash_it_next(&block_it);
    while (block) {
        plugin_pack_block_t next = (plugin_pack_block_t)cpe_hash_it_next(&block_it);
        if (block->m_src_texture == src_texture) {
            plugin_pack_block_free(block);
        }
        block = next;
    }
}

uint32_t plugin_pack_block_hash(const plugin_pack_block_t block) {
    return cpe_hash_md5(&block->m_md5);
}

int plugin_pack_block_eq(const plugin_pack_block_t l, const plugin_pack_block_t r) {
    return cpe_md5_cmp(&l->m_md5, &r->m_md5) == 0;
}

