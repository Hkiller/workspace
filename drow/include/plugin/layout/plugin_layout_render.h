#ifndef PLUGIN_LAYOUT_RENDER_H
#define PLUGIN_LAYOUT_RENDER_H
#include "plugin_layout_types.h"

#ifdef __cplusplus
extern "C" {
#endif

plugin_layout_layout_t plugin_layout_render_layout(plugin_layout_render_t render);
plugin_layout_layout_t plugin_layout_render_set_layout(
    plugin_layout_render_t render, const char * layout_name);

const char * plugin_layout_render_data(plugin_layout_render_t render);
size_t plugin_layout_render_data_len(plugin_layout_render_t render);

int plugin_layout_render_set_data(plugin_layout_render_t render, const char * content);
int plugin_layout_render_set_data_ucs4(plugin_layout_render_t render, uint32_t const * text, size_t text_len);
int plugin_layout_render_set_data_extern(plugin_layout_render_t render, const char * content);
    
ui_vector_2_t plugin_layout_render_size(plugin_layout_render_t render);
void plugin_layout_render_set_size(plugin_layout_render_t render, ui_vector_2_t size);

ui_vector_2_t plugin_layout_render_pos(plugin_layout_render_t render);
void plugin_layout_render_set_pos(plugin_layout_render_t render, ui_vector_2_t size);

void plugin_layout_render_do_layout(plugin_layout_render_t render);

void plugin_layout_render_bound_rt(plugin_layout_render_t render, ui_rect_t rt);
void plugin_layout_render_adj_pos(plugin_layout_render_t render, ui_vector_2_t adj_pos);

uint32_t plugin_layout_render_node_count(plugin_layout_render_t render);
void plugin_layout_render_nodes(plugin_layout_render_t render, plugin_layout_render_node_it_t it);

cpe_str_ucs4_t plugin_layout_render_text_ucs4(mem_allocrator_t alloc, plugin_layout_render_t render);
char * plugin_layout_render_text_utf8(mem_allocrator_t alloc, plugin_layout_render_t render);

int plugin_layout_render_update_text_ucs4(plugin_layout_render_t render, int begin_pos, int end_pos, uint32_t const * text, size_t text_len);
int plugin_layout_render_update_text_utf8(plugin_layout_render_t render, int begin_pos, int end_pos, const char * text);
    
#ifdef __cplusplus
}
#endif

#endif
