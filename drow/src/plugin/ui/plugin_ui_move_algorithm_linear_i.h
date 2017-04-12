#ifndef PLUGIN_UI_ANIMATION_MOVE_ALGORITHM_LINEAR_I_H
#define PLUGIN_UI_ANIMATION_MOVE_ALGORITHM_LINEAR_I_H
#include "plugin_ui_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_ui_move_algorithm_linear {
    float m_speed;
};

int plugin_ui_move_algorithm_linear_regist(plugin_ui_module_t module);
void plugin_ui_move_algorithm_linear_unregist(plugin_ui_module_t module);

#ifdef __cplusplus
}
#endif

#endif
