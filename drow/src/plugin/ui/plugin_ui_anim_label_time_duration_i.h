#ifndef DROW_PLUGIN_UI_ANIM_LABEL_TIME_DURATION_I_H
#define DROW_PLUGIN_UI_ANIM_LABEL_TIME_DURATION_I_H
#include "plugin/ui/plugin_ui_anim_label_time_duration.h"
#include "plugin_ui_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_ui_anim_label_time_duration {
    plugin_ui_module_t m_module;
    float m_cfg_duration;
    uint32_t m_cfg_formator[plugin_ui_anim_label_time_duration_formator_year + 1];
    char * m_cfg_done_res;
    
    /*runtime*/
    float m_worked_duration;
    float m_updated_duration;
};

int plugin_ui_anim_label_time_duration_regist(plugin_ui_module_t module);
void plugin_ui_anim_label_time_duration_unregist(plugin_ui_module_t module);

#ifdef __cplusplus
}
#endif

#endif
