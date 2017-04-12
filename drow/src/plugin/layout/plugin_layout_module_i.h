#ifndef PLUGIN_LAYOUT_MODULE_I_H
#define PLUGIN_LAYOUT_MODULE_I_H
#include "cpe/pal/pal_queue.h"
#include "cpe/utils/memory.h"
#include "cpe/utils/error.h"
#include "cpe/utils/hash.h"
#include "cpe/utils/buffer.h"
#include "gd/app/app_context.h"
#include "plugin/layout/plugin_layout_module.h"
#include "plugin/layout/plugin_layout_font_info.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct plugin_layout_render_group_node * plugin_layout_render_group_node_t;

typedef TAILQ_HEAD(plugin_layout_font_meta_list, plugin_layout_font_meta) plugin_layout_font_meta_list_t;
typedef TAILQ_HEAD(plugin_layout_font_element_list, plugin_layout_font_element) plugin_layout_font_element_list_t;
typedef TAILQ_HEAD(plugin_layout_font_face_list, plugin_layout_font_face) plugin_layout_font_face_list_t;
    
typedef TAILQ_HEAD(plugin_layout_layout_list, plugin_layout_layout) plugin_layout_layout_list_t;
typedef TAILQ_HEAD(plugin_layout_render_node_list, plugin_layout_render_node) plugin_layout_render_node_list_t;
typedef TAILQ_HEAD(plugin_layout_render_group_list, plugin_layout_render_group) plugin_layout_render_group_list_t;
typedef TAILQ_HEAD(plugin_layout_render_group_node_list, plugin_layout_render_group_node) plugin_layout_render_group_node_list_t;

typedef TAILQ_HEAD(plugin_layout_layout_rich_block_list, plugin_layout_layout_rich_block) plugin_layout_layout_rich_block_list_t;

typedef TAILQ_HEAD(plugin_layout_animation_list, plugin_layout_animation) plugin_layout_animation_list_t;

struct plugin_layout_module {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    uint8_t m_debug;
    ui_data_mgr_t m_data_mgr;
    ui_cache_manager_t m_cache_mgr;
    ui_runtime_module_t m_runtime;

    struct plugin_layout_font_id m_default_font_id;
    
    struct cpe_hash_table m_layout_metas;
    uint32_t m_layout_data_capacity;
    
    plugin_layout_layout_list_t m_layouts;
    plugin_layout_layout_list_t m_free_layouts;

    plugin_layout_font_cache_t m_font_cache;
    uint32_t m_max_element_capacity;
    plugin_layout_font_meta_list_t m_font_metas;
    struct cpe_hash_table m_font_faces;
    uint32_t m_element_count;
    uint32_t m_free_element_count;
    plugin_layout_font_element_list_t m_free_font_elements;
    
    uint32_t m_node_count;
    uint32_t m_free_node_count;
    uint32_t m_group_count;
    uint32_t m_free_group_count;
    uint32_t m_group_node_count;
    uint32_t m_free_group_node_count;
    plugin_layout_render_node_list_t m_free_render_nodes;
    plugin_layout_render_group_list_t m_free_render_groups;
    plugin_layout_render_group_node_list_t m_free_render_group_nodes;
    plugin_layout_layout_rich_block_list_t m_free_layout_rich_blocks;
    
    /*Animation */
    uint32_t m_animation_max_capacity;
    struct cpe_hash_table m_animation_metas;
    uint32_t m_max_animation_id;
    struct cpe_hash_table m_animations;
    plugin_layout_animation_list_t m_free_animations;

    /*animation meta*/
    plugin_layout_animation_meta_t m_animation_meta_caret;
    plugin_layout_animation_meta_t m_animation_meta_selection;
    
    struct mem_buffer m_dump_buffer;
};

ptr_int_t plugin_layout_module_tick(void * ctx, ptr_int_t arg, float delta_s);

#ifdef __cplusplus
}
#endif

#endif 
