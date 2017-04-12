#ifndef PLUGIN_UI_ANIM_PROGRESS_BIND_CONTROL_I_H
#define PLUGIN_UI_ANIM_PROGRESS_BIND_CONTROL_I_H
#include "plugin/ui/plugin_ui_anim_progress_bind_control.h"
#include "plugin_ui_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

enum plugin_ui_anim_progress_bind_control_pos_policy {
    plugin_ui_anim_progress_bind_control_near = 0,
    plugin_ui_anim_progress_bind_control_middle = 1,
    plugin_ui_anim_progress_bind_control_far = 2,
};

struct plugin_ui_anim_progress_bind_control {
    plugin_ui_module_t m_module;
    enum plugin_ui_anim_progress_bind_control_pos_policy m_pos_policy;
};

int plugin_ui_anim_progress_bind_control_regist(plugin_ui_module_t module);
void plugin_ui_anim_progress_bind_control_unregist(plugin_ui_module_t module);

#ifdef __cplusplus
}
#endif

#endif
