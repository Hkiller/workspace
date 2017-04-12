#ifndef DROW_LAYOUT_LAYOUT_RICH_BLOCK_H
#define DROW_LAYOUT_LAYOUT_RICH_BLOCK_H
#include "plugin_layout_types.h"

#ifdef __cplusplus
extern "C" {
#endif

plugin_layout_layout_rich_block_t plugin_layout_layout_rich_block_create(plugin_layout_layout_rich_t rich);
void plugin_layout_layout_rich_block_free(plugin_layout_layout_rich_block_t block);

void plugin_layout_layout_rich_block_set_font_id(plugin_layout_layout_rich_block_t block, plugin_layout_font_id_t font_id);
void plugin_layout_layout_rich_block_set_font_draw(plugin_layout_layout_rich_block_t block, plugin_layout_font_draw_t font_draw);

void plugin_layout_layout_rich_block_set_color(plugin_layout_layout_rich_block_t block, ui_color_t color);
void plugin_layout_layout_rich_block_set_size(plugin_layout_layout_rich_block_t block, uint8_t size);
void plugin_layout_layout_rich_block_set_adj_size(plugin_layout_layout_rich_block_t block, int8_t size_adj);
    
int plugin_layout_layout_rich_block_set_context_range(
    plugin_layout_layout_rich_block_t block, const char * begin, const char *  end, uint8_t own);

int plugin_layout_layout_rich_block_set_context(
    plugin_layout_layout_rich_block_t block, const char * data, uint8_t own);
    
#ifdef __cplusplus
}
#endif

#endif

