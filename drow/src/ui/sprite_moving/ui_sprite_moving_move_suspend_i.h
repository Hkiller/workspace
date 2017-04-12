#ifndef UI_SPRITE_MOVING_MOVE_SUSPEND_I_H
#define UI_SPRITE_MOVING_MOVE_SUSPEND_I_H
#include "ui/sprite_moving/ui_sprite_moving_move_suspend.h"
#include "ui_sprite_moving_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_moving_move_suspend {
    ui_sprite_moving_module_t m_module;
};

int ui_sprite_moving_move_suspend_regist(ui_sprite_moving_module_t module);
void ui_sprite_moving_move_suspend_unregist(ui_sprite_moving_module_t module);

#ifdef __cplusplus
}
#endif

#endif
