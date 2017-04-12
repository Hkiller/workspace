#ifndef DROW_PLUGIN_LAYOUT_ANIMATION_H
#define DROW_PLUGIN_LAYOUT_ANIMATION_H
#include "plugin_layout_types.h"

#ifdef __cplusplus
extern "C" {
#endif

plugin_layout_animation_t plugin_layout_animation_create(plugin_layout_render_t render, plugin_layout_animation_meta_t meta);
plugin_layout_animation_t plugin_layout_animation_create_by_type_name(plugin_layout_render_t render, const char * type_name);
void plugin_layout_animation_free(plugin_layout_animation_t animation);

plugin_layout_animation_t plugin_layout_animation_find_first_by_type(plugin_layout_render_t render, const char * type_name);

plugin_layout_render_t plugin_layout_animation_render(plugin_layout_animation_t animation);

uint32_t plugin_layout_animation_id(plugin_layout_animation_t animation);
plugin_layout_animation_t plugin_layout_animation_find(plugin_layout_module_t module, uint32_t animation_id);

void * plugin_layout_animation_data(plugin_layout_animation_t animation);
plugin_layout_animation_t plugin_layout_animation_from_data(void * data);

#ifdef __cplusplus
}
#endif

#endif

