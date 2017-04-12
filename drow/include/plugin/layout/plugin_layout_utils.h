#ifndef DROW_PLUGIN_LAYOUT_UTILS_H
#define DROW_PLUGIN_LAYOUT_UTILS_H
#include "plugin_layout_types.h"

#ifdef __cplusplus
extern "C" {
#endif

int plugin_layout_align_in_rect(
    plugin_layout_render_t render,
    ui_rect_t target_rect, plugin_layout_align_t align);

int plugin_layout_select_line(
    plugin_layout_render_node_t start_node, plugin_layout_render_group_t group);

size_t plugin_layout_render_node_wlen(plugin_layout_render_node_it_t node_it);
cpe_str_ucs4_t plugin_layout_render_nodes_to_ucs4(mem_allocrator_t alloc, plugin_layout_render_node_it_t node_it);
    
size_t plugin_layout_render_node_clen(plugin_layout_render_node_it_t node_it);
char * plugin_layout_render_nodes_to_utf8(mem_allocrator_t alloc, plugin_layout_render_node_it_t node_it);
    
#ifdef __cplusplus
}
#endif

#endif
