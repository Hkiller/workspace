#ifndef UI_SPRITE_MOVING_MOVE_BY_PLAN_I_H
#define UI_SPRITE_MOVING_MOVE_BY_PLAN_I_H
#include "plugin/moving/plugin_moving_control.h"
#include "plugin/moving/plugin_moving_node.h"
#include "ui/sprite_moving/ui_sprite_moving_move_by_plan.h"
#include "ui_sprite_moving_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_moving_move_by_plan {
    ui_sprite_moving_module_t m_module;
    char * m_cfg_res;
    char * m_cfg_node_name;
    char * m_cfg_move_policy;
    char * m_cfg_loop_count;
    
    char * m_node_name;
    ui_sprite_moving_policy_t m_move_policy;
    uint32_t m_loop_count;
    plugin_moving_control_t m_control;
};

int ui_sprite_moving_move_by_plan_regist(ui_sprite_moving_module_t module);
void ui_sprite_moving_move_by_plan_unregist(ui_sprite_moving_module_t module);

#ifdef __cplusplus
}
#endif

#endif
