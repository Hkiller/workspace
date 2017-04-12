#ifndef PLUGIN_UI_EXEC_STEP_CLICK_I_H
#define PLUGIN_UI_EXEC_STEP_CLICK_I_H
#include "plugin_ui_exec_step_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_ui_exec_step_click {
    const char * m_action;
    uint8_t m_is_done;
};

#ifdef __cplusplus
}
#endif

#endif
