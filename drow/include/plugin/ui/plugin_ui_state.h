#ifndef DROW_PLUGIN_UI_STATE_H
#define DROW_PLUGIN_UI_STATE_H
#include "plugin_ui_types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_ui_state_it {
    plugin_ui_state_t (*next)(struct plugin_ui_state_it * it);
    char m_data[64];
};
    
plugin_ui_state_t plugin_ui_state_create(plugin_ui_phase_t phase, const char * state_name);
void plugin_ui_state_free(plugin_ui_state_t state);

plugin_ui_state_t plugin_ui_state_find(plugin_ui_phase_t phase, const char * state_name);

plugin_ui_phase_t plugin_ui_state_phase(plugin_ui_state_t state);
const char * plugin_ui_state_name(plugin_ui_state_t state);

void * plugin_ui_state_data(plugin_ui_state_t state);
plugin_package_group_t plugin_ui_state_packages(plugin_ui_state_t state);
plugin_package_group_t plugin_ui_state_packages_check_create(plugin_ui_state_t state);    
    
void plugin_ui_state_navigations_to(plugin_ui_state_t state, plugin_ui_navigation_it_t navigation_it);
void plugin_ui_state_navigations_from(plugin_ui_state_t state, plugin_ui_navigation_it_t navigation_it);
void plugin_ui_state_navigations_as_loading(plugin_ui_state_t state, plugin_ui_navigation_it_t navigation_it);
void plugin_ui_state_navigations_as_back(plugin_ui_state_t state, plugin_ui_navigation_it_t navigation_it);

plugin_ui_navigation_t plugin_ui_state_auto_execute(plugin_ui_state_t state);
void plugin_ui_state_set_auto_execute(plugin_ui_state_t state, plugin_ui_navigation_t navigation);

#define plugin_ui_state_it_next(it) ((it)->next ? (it)->next(it) : NULL)

#ifdef __cplusplus
}
#endif

#endif

