#ifndef PLUGIN_UI_ANIM_FRAME_BIND_I_H
#define PLUGIN_UI_ANIM_FRAME_BIND_I_H
#include "plugin/ui/plugin_ui_anim_frame_bind.h"
#include "plugin_ui_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_ui_anim_frame_bind {
    /*config*/
    char * m_cfg_target;
    uint8_t m_remove;
};

int plugin_ui_anim_frame_bind_regist(plugin_ui_module_t module);
void plugin_ui_anim_frame_bind_unregist(plugin_ui_module_t module);
    
#ifdef __cplusplus
}
#endif

#endif
