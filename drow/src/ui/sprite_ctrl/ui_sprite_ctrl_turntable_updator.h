#ifndef UI_SPRITE_CTRL_TURNTABLE_UPDATOR_H
#define UI_SPRITE_CTRL_TURNTABLE_UPDATOR_H
#include "ui_sprite_ctrl_turntable_member_i.h"
#include "render/utils/ui_percent_decorator.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ui_sprite_ctrl_turntable_updator * ui_sprite_ctrl_turntable_updator_t;

struct ui_sprite_ctrl_turntable_updator {
    uint32_t m_curent_op_id;
    struct ui_percent_decorator m_decorator;
    float m_origin_angle;
    float m_target_angle;
    float m_max_speed;
    float m_duration;
    float m_runing_time;
};

void ui_sprite_ctrl_turntable_updator_stop(ui_sprite_ctrl_turntable_updator_t updator, ui_sprite_ctrl_turntable_member_t member);
void ui_sprite_ctrl_turntable_updator_set_max_speed(ui_sprite_ctrl_turntable_updator_t updator, float max_speed);

void ui_sprite_ctrl_turntable_updator_set_angle(ui_sprite_ctrl_turntable_updator_t updator, ui_sprite_ctrl_turntable_member_t member, float angle);
void ui_sprite_ctrl_turntable_updator_update(ui_sprite_ctrl_turntable_updator_t updator, ui_sprite_ctrl_turntable_member_t member, float delta);
uint8_t ui_sprite_ctrl_turntable_updator_is_runing(ui_sprite_ctrl_turntable_updator_t updator);

#ifdef __cplusplus
}
#endif

#endif
