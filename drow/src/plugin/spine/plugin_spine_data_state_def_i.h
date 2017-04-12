#ifndef PLUGIN_SPINE_DATA_STATE_DEF_I_H
#define PLUGIN_SPINE_DATA_STATE_DEF_I_H
#include "plugin/spine/plugin_spine_data_state_def.h"
#include "plugin_spine_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef TAILQ_HEAD(plugin_spine_data_part_list, plugin_spine_data_part) plugin_spine_data_part_list_t;
typedef TAILQ_HEAD(plugin_spine_data_part_state_list, plugin_spine_data_part_state) plugin_spine_data_part_state_list_t;
typedef TAILQ_HEAD(plugin_spine_data_part_transition_list, plugin_spine_data_part_transition) plugin_spine_data_part_transition_list_t;

struct plugin_spine_data_state_def {
    plugin_spine_module_t m_module;
    ui_data_src_t m_src;
    uint16_t m_part_count;
    plugin_spine_data_part_list_t m_parts;
};

struct plugin_spine_data_part {
    plugin_spine_data_state_def_t m_def;
    TAILQ_ENTRY(plugin_spine_data_part) m_next;
    uint16_t m_state_count;
    plugin_spine_data_part_state_list_t m_states;
    uint16_t m_transition_count;
    plugin_spine_data_part_transition_list_t m_transitions;
    SPINE_PART m_data;
};

struct plugin_spine_data_part_state {
    plugin_spine_data_part_t m_part;
    TAILQ_ENTRY(plugin_spine_data_part_state) m_next;
    SPINE_PART_STATE m_data;
};

struct plugin_spine_data_part_transition {
    plugin_spine_data_part_t m_part;
    TAILQ_ENTRY(plugin_spine_data_part_transition) m_next;
    SPINE_PART_TRANSITION m_data;
};

int plugin_spine_data_state_def_bin_save(void * ctx, ui_data_mgr_t mgr, ui_data_src_t src, const char * root, error_monitor_t em);
int plugin_spine_data_state_def_bin_load(void * ctx, ui_data_mgr_t mgr, ui_data_src_t src, error_monitor_t em);
int plugin_spine_data_state_def_bin_rm(void * ctx, ui_data_mgr_t mgr, ui_data_src_t src, const char * root, error_monitor_t em);

#ifdef __cplusplus
}
#endif

#endif
