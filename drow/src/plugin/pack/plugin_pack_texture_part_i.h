#ifndef PLUGIN_PACK_TEXTURE_PART_I_H
#define PLUGIN_PACK_TEXTURE_PART_I_H
#include "plugin_pack_texture_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_pack_texture_part {
    plugin_pack_texture_t m_texture;
    TAILQ_ENTRY(plugin_pack_texture_part) m_next;
    binpack_maxrects_ctx_t m_binpack;
    plugin_pack_block_list_t m_blocks;
};

plugin_pack_texture_part_t plugin_pack_texture_part_create(plugin_pack_texture_t texture);
void plugin_pack_texture_part_free(plugin_pack_texture_part_t texture_part);

int plugin_pack_texture_part_commit(plugin_pack_texture_part_t part, const char * path);

plugin_pack_texture_part_t
plugin_pack_texture_alloc(
    plugin_pack_texture_t texture, ui_cache_pixel_buf_rect_t src_rect, binpack_rect_t o_placed_pack);
    
#ifdef __cplusplus
}
#endif

#endif
