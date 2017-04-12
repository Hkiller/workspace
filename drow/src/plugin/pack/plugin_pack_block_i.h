#ifndef PLUGIN_PACK_BLOCK_I_H
#define PLUGIN_PACK_BLOCK_I_H
#include "cpe/utils/md5.h"
#include "render/cache/ui_cache_pixel_buf_manip.h"
#include "plugin_pack_texture_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_pack_block {
    plugin_pack_texture_t m_texture;
    plugin_pack_texture_part_t m_part;
    TAILQ_ENTRY(plugin_pack_block) m_next_for_part;
    cpe_hash_entry m_hh;
    struct cpe_md5_value m_md5;
    struct ui_cache_pixel_buf_rect m_to_rect;
    ui_cache_res_t m_src_texture;
    struct ui_cache_pixel_buf_rect m_src_rect;
    plugin_pack_block_ref_list_t m_refs;
};

plugin_pack_block_t
plugin_pack_block_create(
    plugin_pack_texture_t texture, plugin_pack_texture_part_t part,
    cpe_md5_value_t md5, ui_cache_res_t src_texture, ui_cache_pixel_buf_rect_t src_rect);
    
plugin_pack_block_t plugin_pack_block_find(plugin_pack_texture_t texture, cpe_md5_value_t md5);
void plugin_pack_block_free(plugin_pack_block_t block);

void plugin_pack_block_free_all(plugin_pack_texture_t texture);
void plugin_pack_block_free_by_src_texture(plugin_pack_texture_t texture, ui_cache_res_t src_texture);
    
uint32_t plugin_pack_block_hash(const plugin_pack_block_t block);
int plugin_pack_block_eq(const plugin_pack_block_t l, const plugin_pack_block_t r);
    
#ifdef __cplusplus
}
#endif

#endif
