#ifndef PLUGIN_SCROLLMAP_DATA_RANGE_I_H
#define PLUGIN_SCROLLMAP_DATA_RANGE_I_H
#include "plugin/scrollmap/plugin_scrollmap_range.h"
#include "plugin_scrollmap_layer_i.h"
#include "plugin_scrollmap_source_i.h"
#include "plugin_scrollmap_block_i.h"
#include "plugin_scrollmap_script_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_scrollmap_range {
    plugin_scrollmap_layer_t m_layer;
    TAILQ_ENTRY(plugin_scrollmap_range) m_next_for_layer;
    plugin_scrollmap_source_t m_source;
    TAILQ_ENTRY(plugin_scrollmap_range) m_next_for_source;

    plugin_scrollmap_block_list_t m_blocks;
    plugin_scrollmap_script_list_t m_scripts;

    float m_start_pos;
    float m_logic_pos;
    
    plugin_scrollmap_range_state_t m_state;
    plugin_scrollmap_data_layer_t m_layer_data;

    float m_loop_begin;
    float m_loop_end;
    uint16_t m_is_first_loop;
    uint16_t m_loop_count;
    plugin_scrollmap_data_block_t m_loop_blocks_begin;
    plugin_scrollmap_data_script_t m_loop_scripts_begin;
    
    /*runtime data*/
    plugin_scrollmap_data_block_t m_next_block;
    plugin_scrollmap_data_script_t m_next_script;
};

void plugin_scrollmap_range_set_state(plugin_scrollmap_range_t range, plugin_scrollmap_range_state_t state);
void plugin_scrollmap_range_cancel_loop(plugin_scrollmap_range_t range);
    
#ifdef __cplusplus
}
#endif

#endif
