#ifndef PLUGIN_UI_EXEC_STEP_MOVE_I_H
#define PLUGIN_UI_EXEC_STEP_MOVE_I_H
#include "plugin_ui_exec_step_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_ui_exec_step_move {
    const char * m_move_control;
    const char * m_to_control;
    uint8_t m_is_error;
    uint8_t m_is_done;
};

#ifdef __cplusplus
}
#endif

#endif
