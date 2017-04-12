#ifndef PLUGIN_UI_STATE_I_H
#define PLUGIN_UI_STATE_I_H
#include "plugin/ui/plugin_ui_state.h"
#include "plugin_ui_phase_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_ui_state {
    plugin_ui_phase_t m_phase;
    struct cpe_hash_entry m_hh_for_phase;

    char m_name[64];
    plugin_package_group_t m_packages;
    plugin_ui_navigation_t m_auto_execute;
    plugin_ui_navigation_list_t m_navigations_to;
    plugin_ui_navigation_list_t m_navigations_from;
    plugin_ui_navigation_list_t m_navigations_as_loading;
    plugin_ui_navigation_list_t m_navigations_as_back;    
};

void plugin_ui_state_free_all(const plugin_ui_phase_t phase);

uint32_t plugin_ui_state_hash(const plugin_ui_state_t state);
int plugin_ui_state_eq(const plugin_ui_state_t l, const plugin_ui_state_t r);
    
#ifdef __cplusplus
}
#endif

#endif
