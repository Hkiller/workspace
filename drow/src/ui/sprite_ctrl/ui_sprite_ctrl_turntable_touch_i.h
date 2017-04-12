#ifndef UI_SPRITE_CTRL_TURNTABLE_TOUCH_I_H
#define UI_SPRITE_CTRL_TURNTABLE_TOUCH_I_H
#include "ui/sprite_ctrl/ui_sprite_ctrl_turntable_touch.h"
#include "render/utils/ui_percent_decorator.h"
#include "ui_sprite_ctrl_module_i.h"
#include "ui_sprite_ctrl_turntable_updator.h"

#ifdef __cplusplus
extern "C" {
#endif

enum ui_sprite_ctrl_turntable_touch_state {
    ui_sprite_ctrl_turntable_touch_state_idle
    , ui_sprite_ctrl_turntable_touch_state_move
    , ui_sprite_ctrl_turntable_touch_state_move_by_speed
};


struct ui_sprite_ctrl_turntable_touch {
    ui_sprite_ctrl_module_t m_module;
    struct ui_sprite_ctrl_turntable_updator m_updator;
    struct ui_percent_decorator m_decorator;
    float m_focuse_angle_range;
    enum ui_sprite_ctrl_turntable_touch_state m_state;
    ui_vector_2 m_begin_pos;
};

int ui_sprite_ctrl_turntable_touch_regist(ui_sprite_ctrl_module_t module);
void ui_sprite_ctrl_turntable_touch_unregist(ui_sprite_ctrl_module_t module);

#ifdef __cplusplus
}
#endif

#endif
