#ifndef PLUGIN_PACK_TEXTURE_I_H
#define PLUGIN_PACK_TEXTURE_I_H
#include "plugin/pack/plugin_pack_texture.h"
#include "plugin_pack_packer_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_pack_texture {
    plugin_pack_packer_t m_packer;
    char * m_path;
    TAILQ_ENTRY(plugin_pack_texture) m_next;
    uint32_t m_part_count;
    plugin_pack_texture_part_list_t m_parts;
    struct cpe_hash_table m_blocks;
};

plugin_pack_texture_t
plugin_pack_texture_create(plugin_pack_packer_t packer, const char * path);

void plugin_pack_texture_free(plugin_pack_texture_t texture);

int plugin_pack_texture_commit(plugin_pack_texture_t texture);
    
plugin_pack_block_ref_t
plugin_pack_texture_add_block_ref(
    plugin_pack_texture_t texture, ui_data_src_t src, ui_ed_obj_t ed_obj,
    ui_cache_res_t res, ui_cache_pixel_buf_t buf, ui_cache_pixel_buf_rect_t rect);

plugin_pack_block_t
plugin_pack_texture_place(
    plugin_pack_texture_t texture, cpe_md5_value_t md5, ui_cache_res_t src_texture, ui_cache_pixel_buf_rect_t src_rect);
    
#ifdef __cplusplus
}
#endif

#endif
