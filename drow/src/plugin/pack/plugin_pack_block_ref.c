#include "plugin_pack_block_ref_i.h"

plugin_pack_block_ref_t
plugin_pack_block_ref_create(plugin_pack_block_t block, ui_ed_obj_t ed_obj) {
    plugin_pack_module_t module = block->m_texture->m_packer->m_module;
    plugin_pack_block_ref_t block_ref;

    block_ref = mem_alloc(module->m_alloc, sizeof(struct plugin_pack_block_ref));
    if (block_ref == NULL) {
        CPE_ERROR(module->m_em, "plugin_pack_block_ref_create: allock fail!");
        return NULL;
    }

    block_ref->m_block = block;
    block_ref->m_ed_obj = ed_obj;
    TAILQ_INSERT_TAIL(&block->m_refs, block_ref, m_next);
    
    return block_ref;
}

void plugin_pack_block_ref_free(plugin_pack_block_ref_t block_ref) {
    plugin_pack_module_t module = block_ref->m_block->m_texture->m_packer->m_module;

    TAILQ_REMOVE(&block_ref->m_block->m_refs, block_ref, m_next);
    mem_free(module->m_alloc, block_ref);
}
