#ifndef PLUGIN_UI_CONTROL_ACTION_SLOTS_I_H
#define PLUGIN_UI_CONTROL_ACTION_SLOTS_I_H
#include "plugin_ui_control_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_ui_control_action_slots {
    union {
        plugin_ui_control_action_list_t m_actions[plugin_ui_event_max - plugin_ui_event_min];
        plugin_ui_control_action_slots_t m_next;
    };
};

int plugin_ui_control_action_slots_create(plugin_ui_control_t control);
void plugin_ui_control_action_slots_free(plugin_ui_control_t control, plugin_ui_control_action_slots_t slots);
void plugin_ui_control_action_slots_real_free(plugin_ui_env_t env, plugin_ui_control_action_slots_t slots);

#ifdef __cplusplus
}
#endif

#endif
