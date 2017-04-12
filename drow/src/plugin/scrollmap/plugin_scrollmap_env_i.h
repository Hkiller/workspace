#ifndef PLUGIN_SCROLLMAP_CONTROL_I_H
#define PLUGIN_SCROLLMAP_CONTROL_I_H
#include "cpe/pal/pal_queue.h"
#include "cpe/dr/dr_types.h"
#include "cpe/utils/buffer.h"
#include "cpe/utils/hash.h"
#include "render/utils/ui_rect.h"
#include "plugin/scrollmap/plugin_scrollmap_env.h"
#include "plugin/scrollmap/plugin_scrollmap_script.h"
#include "plugin_scrollmap_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct plugin_scrollmap_update_ctx * plugin_scrollmap_update_ctx_t;
typedef struct plugin_scrollmap_tile * plugin_scrollmap_tile_t;
typedef TAILQ_HEAD(plugin_scrollmap_source_list, plugin_scrollmap_source) plugin_scrollmap_source_list_t;
typedef TAILQ_HEAD(plugin_scrollmap_range_list, plugin_scrollmap_range) plugin_scrollmap_range_list_t;
typedef TAILQ_HEAD(plugin_scrollmap_layer_list, plugin_scrollmap_layer) plugin_scrollmap_layer_list_t;
typedef TAILQ_HEAD(plugin_scrollmap_block_list, plugin_scrollmap_block) plugin_scrollmap_block_list_t;
typedef TAILQ_HEAD(plugin_scrollmap_script_list, plugin_scrollmap_script) plugin_scrollmap_script_list_t;
typedef TAILQ_HEAD(plugin_scrollmap_obj_list, plugin_scrollmap_obj) plugin_scrollmap_obj_list_t;
typedef TAILQ_HEAD(plugin_scrollmap_team_list, plugin_scrollmap_team) plugin_scrollmap_team_list_t;
typedef TAILQ_HEAD(plugin_scrollmap_tile_list, plugin_scrollmap_tile) plugin_scrollmap_tile_list_t;

struct plugin_scrollmap_env {
    plugin_scrollmap_module_t m_module;
    uint8_t m_debug;
    plugin_scrollmap_moving_way_t m_moving_way;
    ui_vector_2 m_base_size;
    ui_vector_2 m_runing_size;
    ui_vector_2 m_logic_size_adj;
    plugin_scrollmap_resize_policy_t m_resize_policy_x;
    plugin_scrollmap_resize_policy_t m_resize_policy_y;
    float m_move_speed;

    uint8_t m_is_suspend;
    ui_rect m_view_pos;
    plugin_scrollmap_source_list_t m_sources;
    plugin_scrollmap_layer_list_t m_layers;
    struct cpe_hash_table m_tiles;
    plugin_scrollmap_tile_list_t m_free_tiles;
    plugin_scrollmap_block_list_t m_free_blocks;
    plugin_scrollmap_script_list_t m_free_scripts;

    struct cpe_hash_table m_script_executors;
    
    uint16_t m_obj_count;
    plugin_scrollmap_obj_list_t m_objs;
    plugin_scrollmap_obj_list_t m_free_objs;

    plugin_moving_env_t m_moving_env;
    uint16_t m_team_count;
    uint16_t m_team_max_id;
    plugin_scrollmap_team_list_t m_teams;
    plugin_scrollmap_team_list_t m_free_teams;

    void * m_obj_factory_ctx;
    uint32_t m_obj_capacity;
    plugin_scrollmap_obj_name_fun_t m_obj_name;
    plugin_scrollmap_obj_on_init_fun_t m_obj_on_init;
    plugin_scrollmap_obj_on_update_fun_t m_obj_on_update;
    plugin_scrollmap_obj_on_event_fun_t m_obj_on_event;
    plugin_scrollmap_obj_on_destory_fun_t m_obj_on_destory;

    void * m_script_check_ctx;
    plugin_scrollmap_script_check_fun_t m_script_check_fun;
    
    struct mem_buffer m_dump_buffer;
};

#ifdef __cplusplus
}
#endif

#endif
