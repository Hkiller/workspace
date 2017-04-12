#ifndef UI_SPRITE_CTRL_TURNTABLE_ACTIVE_I_H
#define UI_SPRITE_CTRL_TURNTABLE_ACTIVE_I_H
#include "ui/sprite_ctrl/ui_sprite_ctrl_turntable_active.h"
#include "render/utils/ui_percent_decorator.h"
#include "ui_sprite_ctrl_module_i.h"
#include "ui_sprite_ctrl_turntable_updator.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_ctrl_turntable_active {
    ui_sprite_ctrl_module_t m_module;
    struct ui_sprite_ctrl_turntable_updator m_updator;
    struct ui_percent_decorator m_decorator;
};

int ui_sprite_ctrl_turntable_active_regist(ui_sprite_ctrl_module_t module);
void ui_sprite_ctrl_turntable_active_unregist(ui_sprite_ctrl_module_t module);

#ifdef __cplusplus
}
#endif

#endif
