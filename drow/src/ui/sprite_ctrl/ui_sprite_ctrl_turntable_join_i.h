#ifndef UI_SPRITE_CTRL_TURNTABLE_JOIN_I_H
#define UI_SPRITE_CTRL_TURNTABLE_JOIN_I_H
#include "ui/sprite_ctrl/ui_sprite_ctrl_turntable_join.h"
#include "ui_sprite_ctrl_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_ctrl_turntable_join {
    ui_sprite_ctrl_module_t m_module;
    char m_turntable_name[64];
};

int ui_sprite_ctrl_turntable_join_regist(ui_sprite_ctrl_module_t module);
void ui_sprite_ctrl_turntable_join_unregist(ui_sprite_ctrl_module_t module);

#ifdef __cplusplus
}
#endif

#endif
