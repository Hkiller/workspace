#ifndef DROW_PLUGIN_PACK_TEXTURE_H
#define DROW_PLUGIN_PACK_TEXTURE_H
#include "plugin_pack_types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_pack_texture_it {
    plugin_pack_texture_t (*next)(struct plugin_pack_texture_it * it);
    char m_data[64];
};

ui_cache_res_t plugin_pack_texture_cache_texture(plugin_pack_texture_t pack_texture);
    
void plugin_pack_packer_textures(plugin_pack_texture_it_t it, plugin_pack_packer_t packer);
    
#define plugin_pack_texture_it_next(it) ((it)->next ? (it)->next(it) : NULL)
    
#ifdef __cplusplus
}
#endif

#endif

