#ifndef PLUGIN_MASK_MANIP_BLOCK_BUILDER_I_H
#define PLUGIN_MASK_MANIP_BLOCK_BUILDER_I_H
#include "plugin_mask_manip_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_mask_block_builder {
    plugin_mask_manip_t m_manip;
    ui_cache_pixel_field_t m_source;
    int32_t m_x;
    int32_t m_y;
    uint32_t m_width;
    uint32_t m_height;
    uint32_t m_lt_x;
    uint32_t m_lt_y;
    uint32_t m_rb_x;
    uint32_t m_rb_y;
    uint8_t m_have_data;
    uint32_t * m_buf;
};

plugin_mask_block_builder_t
plugin_mask_block_builder_create(
    plugin_mask_manip_t manip, ui_cache_pixel_field_t source,
    int32_t x, int32_t y, uint32_t width, uint32_t height);
void plugin_mask_block_builder_free(plugin_mask_block_builder_t builder);

int plugin_mask_block_builder_create_block(
    plugin_mask_block_builder_t builder, ui_data_src_t mask_src, plugin_mask_data_t mask_data, const char * name);
    
int plugin_mask_block_builder_place_img_block(plugin_mask_block_builder_t builder, int32_t x, int32_t y, ui_data_img_block_t img_block);
int plugin_mask_block_builder_place_frame(plugin_mask_block_builder_t builder, int32_t x, int32_t y, ui_data_frame_t frame);

#ifdef __cplusplus
}
#endif

#endif 
