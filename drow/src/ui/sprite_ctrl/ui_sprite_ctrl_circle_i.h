#ifndef UI_SPRITE_CTRL_CIRCLE_I_H
#define UI_SPRITE_CTRL_CIRCLE_I_H
#include "ui/sprite_ctrl/ui_sprite_ctrl_circle.h"
#include "ui_sprite_ctrl_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_ctrl_circle {
    UI_SPRITE_CTRL_CIRCLE_STATE m_state;
    ui_sprite_ctrl_module_t m_module;
    uint8_t m_do_rotate;
    uint8_t m_do_scale;
    uint8_t m_negative;
    char * m_on_begin;
    char * m_on_move;
    char * m_on_done;
    char * m_on_cancel;
    float m_screen_min;
    float m_screen_max;
    float m_logic_min;
    float m_logic_max;
    float m_angle_min;
    float m_angle_max;
    float m_logic_base;
    
    float m_max_distance;
    float m_cancel_distance;

    float m_keep_send_span;
    uint8_t m_is_working;
    float m_last_send_time;
};

int ui_sprite_ctrl_circle_regist(ui_sprite_ctrl_module_t module);
void ui_sprite_ctrl_circle_unregist(ui_sprite_ctrl_module_t module);

#ifdef __cplusplus
}
#endif

#endif
