#ifndef DROW_PLUGIN_UI_PHASE_H
#define DROW_PLUGIN_UI_PHASE_H
#include "plugin_ui_types.h"

#ifdef __cplusplus
extern "C" {
#endif

plugin_ui_phase_t plugin_ui_phase_create(plugin_ui_env_t env, const char * phase_name);
void plugin_ui_phase_free(plugin_ui_phase_t phase);

plugin_ui_phase_t plugin_ui_phase_find(plugin_ui_env_t env, const char * phase_name);

const char * plugin_ui_phase_name(plugin_ui_phase_t phase);

uint8_t plugin_ui_phase_fps(plugin_ui_phase_t phase);
void plugin_ui_phase_set_fps(plugin_ui_phase_t phase, uint8_t fps);

plugin_package_package_t plugin_ui_phase_package(plugin_ui_phase_t phase);
    
void * plugin_ui_phase_data(plugin_ui_phase_t phase);

int plugin_ui_phase_set_init_state(
    plugin_ui_phase_t phase, const char * init_state_name, const char * inint_call_state);

void plugin_ui_phase_states(plugin_ui_phase_t phase, plugin_ui_state_it_t state_it);

uint8_t plugin_ui_phase_is_use_page(plugin_ui_phase_t phase, plugin_ui_page_t page);

#ifdef __cplusplus
}
#endif

#endif

