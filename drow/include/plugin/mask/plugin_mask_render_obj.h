#ifndef PLUGIN_MASK_RENDER_OBJ_H
#define PLUGIN_MASK_RENDER_OBJ_H
#include "plugin_mask_types.h"

#ifdef __cplusplus
extern "C" {
#endif

plugin_mask_data_block_t plugin_mask_render_obj_data_block(plugin_mask_render_obj_t obj);
void plugin_mask_render_obj_set_data_block(plugin_mask_render_obj_t obj, plugin_mask_data_block_t data_block);

void plugin_mask_render_obj_set_color(plugin_mask_render_obj_t obj, ui_color_t color);
ui_color_t plugin_mask_render_obj_color(plugin_mask_render_obj_t obj);

#ifdef __cplusplus
}
#endif

#endif
