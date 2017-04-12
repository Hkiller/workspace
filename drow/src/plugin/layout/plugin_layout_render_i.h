#ifndef PLUGIN_LAYOUT_RENDER_I_H
#define PLUGIN_LAYOUT_RENDER_I_H
#include "render/utils/ui_vector_2.h"
#include "plugin/layout/plugin_layout_render.h"
#include "plugin_layout_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_layout_render {
    plugin_layout_module_t m_module;
    plugin_layout_layout_t m_layout;
    char m_name[64];
    uint32_t m_node_count;
    plugin_layout_render_node_list_t m_nodes;
    plugin_layout_render_group_list_t m_groups;
    plugin_layout_animation_list_t m_animations;
    ui_vector_2 m_pos;
    ui_vector_2 m_size;
    const char * m_data;
    size_t m_data_len;
    uint8_t m_data_manage;
    uint8_t m_need_update;
};

int plugin_layout_render_register(plugin_layout_module_t module);
void plugin_layout_render_unregister(plugin_layout_module_t module);

void plugin_layout_render_clear_nodes(plugin_layout_render_t render);

void plugin_layout_render_render_element(
    ui_rect_t target_rect, ui_rect_t texture_rect,
    ui_cache_res_t texture, ui_runtime_render_texture_filter_t filter,
    ui_runtime_render_program_buildin_t program, ui_color_t blend_color, 
    ui_runtime_render_t render, ui_runtime_render_cmd_t * batch_cmd,
    ui_rect_t clip_rect,
    ui_runtime_render_second_color_t second_color, ui_transform_t transform);
    
#ifdef __cplusplus
}
#endif

#endif
