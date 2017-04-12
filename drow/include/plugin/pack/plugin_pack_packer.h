#ifndef DROW_PLUGIN_PACK_PACKER_H
#define DROW_PLUGIN_PACK_PACKER_H
#include "plugin_pack_types.h"

#ifdef __cplusplus
extern "C" {
#endif

plugin_pack_packer_t plugin_pack_packer_create(plugin_pack_module_t module, const char * target_path);
void plugin_pack_packer_free(plugin_pack_packer_t packer);

uint8_t plugin_pack_packer_texture_span(plugin_pack_packer_t packer);
void plugin_pack_packer_texture_set_span(plugin_pack_packer_t packer, uint8_t span);

uint32_t plugin_pack_packer_texture_limit_width(plugin_pack_packer_t packer);
void plugin_pack_packer_texture_set_limit_width(plugin_pack_packer_t packer, uint32_t limit_width);
uint32_t plugin_pack_packer_texture_limit_height(plugin_pack_packer_t packer);
void plugin_pack_packer_texture_set_limit_height(plugin_pack_packer_t packer, uint32_t limit_height);
    
const char * plugin_pack_packer_default_language(plugin_pack_packer_t packer);
void plugin_pack_packer_set_default_language(plugin_pack_packer_t packer, const char * language);

ui_data_src_group_t plugin_pack_packer_input_srcs(plugin_pack_packer_t packer);
ui_cache_group_t plugin_pack_packer_pack_textures(plugin_pack_packer_t packer);
    
ui_data_src_group_t plugin_pack_packer_packed_srcs(plugin_pack_packer_t packer);
ui_cache_group_t plugin_pack_packer_generated_textures(plugin_pack_packer_t packer);

int plugin_pack_packer_pack(plugin_pack_packer_t packer);

#ifdef __cplusplus
}
#endif

#endif

