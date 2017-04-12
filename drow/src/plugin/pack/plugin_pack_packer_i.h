#ifndef PLUGIN_PACK_PACKER_I_H
#define PLUGIN_PACK_PACKER_I_H
#include "cpe/utils/hash.h"
#include "plugin/pack/plugin_pack_packer.h"
#include "plugin_pack_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_pack_packer {
    plugin_pack_module_t m_module;
    ui_data_src_group_t m_input_srcs;
    ui_cache_group_t m_pack_textures;
    uint8_t m_texture_span;
    uint32_t m_limit_width;
    uint32_t m_limit_height;
    
    ui_data_src_group_t m_packed_srcs;
    ui_cache_group_t m_generated_textures;
    char m_default_language_name[32];
    plugin_pack_language_t m_default_language;
    uint8_t m_language_count;
    plugin_pack_language_list_t m_languages;
    plugin_pack_texture_list_t m_textures;
    plugin_pack_texture_t m_common_texture;
};

int plugin_pack_packer_load_module(plugin_pack_texture_t texture, ui_data_src_t src);
void plugin_pack_packer_update_img_block(plugin_pack_block_ref_t block_ref, const char * path);
        
int plugin_pack_packer_load_particle(plugin_pack_texture_t texture, ui_data_src_t src);
void plugin_pack_packer_update_particle_emitter(plugin_pack_block_ref_t block_ref, const char * path);

void plugin_pack_packer_remove_pack_texture(plugin_pack_packer_t packer, ui_cache_res_t res);
    
/*utils*/    
int plugin_pack_packer_load_src_texture(
    plugin_pack_packer_t packer, ui_data_src_t src, const char * path,
    ui_cache_res_t * output_res, ui_cache_pixel_buf_t * output_buf);
    
#ifdef __cplusplus
}
#endif

#endif
