#ifndef UI_SPRITE_MOVING_MOVE_SET_TO_BEGIN_I_H
#define UI_SPRITE_MOVING_MOVE_SET_TO_BEGIN_I_H
#include "plugin/moving/plugin_moving_control.h"
#include "plugin/moving/plugin_moving_node.h"
#include "ui/sprite_moving/ui_sprite_moving_move_set_to_begin.h"
#include "ui_sprite_moving_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_moving_move_set_to_begin {
    ui_sprite_moving_module_t m_module;
    char m_plan_res[64];
    char m_node_name[64];
    ui_sprite_moving_policy_t m_move_policy;
};

int ui_sprite_moving_move_set_to_begin_regist(ui_sprite_moving_module_t module);
void ui_sprite_moving_move_set_to_begin_unregist(ui_sprite_moving_module_t module);

#ifdef __cplusplus
}
#endif

#endif
