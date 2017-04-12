#ifndef DROW_LAYOUT_LAYOUT_H
#define DROW_LAYOUT_LAYOUT_H
#include "plugin_layout_types.h"

#ifdef __cplusplus
extern "C" {
#endif

plugin_layout_layout_t
plugin_layout_layout_create(plugin_layout_render_t render, plugin_layout_layout_meta_t meta);

void plugin_layout_layout_free(plugin_layout_layout_t layout);

int plugin_layout_layout_setup(plugin_layout_layout_t layout, char * arg_buf_will_change);

void * plugin_layout_layout_data(plugin_layout_layout_t layout);
plugin_layout_layout_t plugin_layout_layout_from_data(void * data);

plugin_layout_render_t plugin_layout_layout_render(plugin_layout_layout_t layout);
plugin_layout_layout_meta_t plugin_layout_layout_meta(plugin_layout_layout_t layout);
const char * plugin_layout_layout_meta_name(plugin_layout_layout_t layout);

#ifdef __cplusplus
}
#endif

#endif

