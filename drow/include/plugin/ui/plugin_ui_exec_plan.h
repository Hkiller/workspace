#ifndef DROW_PLUGIN_UI_EXEC_PLAN_H
#define DROW_PLUGIN_UI_EXEC_PLAN_H
#include "plugin_ui_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum plugin_ui_exec_action_type {
    plugin_ui_exec_action_click,
    plugin_ui_exec_action_move,
    plugin_ui_exec_action_noop,
} plugin_ui_exec_action_type_t;

typedef enum plugin_ui_exec_state_type {
    plugin_ui_exec_success,
    plugin_ui_exec_fail,
    plugin_ui_exec_processing,
} plugin_ui_exec_state_type_t;
    
struct plugin_ui_exec_action {
    plugin_ui_exec_action_type_t m_type;
    union {
        struct {
            plugin_ui_control_t m_click_control;
        } m_click;
        struct {
            plugin_ui_control_t m_move_control;
            plugin_ui_control_t m_to_control;
        } m_move;
    };
};
        
plugin_ui_exec_plan_t plugin_ui_exec_plan_create(plugin_ui_env_t env);
void plugin_ui_exec_plan_free(plugin_ui_exec_plan_t exec_plan);

plugin_ui_aspect_t plugin_ui_exec_plan_aspect(plugin_ui_exec_plan_t exec_plan);

plugin_ui_popup_t plugin_ui_exec_plan_base_popup(plugin_ui_exec_plan_t exec_plan);
void plugin_ui_exec_plan_set_base_popup(plugin_ui_exec_plan_t exec_plan, plugin_ui_popup_t popup);
    
plugin_ui_exec_action_t plugin_ui_exec_plan_next_action(plugin_ui_exec_plan_t exec_plan);
plugin_ui_exec_state_type_t plugin_ui_exec_plan_state(plugin_ui_exec_plan_t exec_plan);

#ifdef __cplusplus
}
#endif

#endif

