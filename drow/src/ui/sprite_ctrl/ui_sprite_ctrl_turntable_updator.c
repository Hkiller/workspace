#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/math_ex.h"
#include "ui_sprite_ctrl_turntable_updator.h"
#include "ui_sprite_ctrl_turntable_i.h"
#include "ui_sprite_ctrl_turntable_member_i.h"

void ui_sprite_ctrl_turntable_updator_set_max_speed(ui_sprite_ctrl_turntable_updator_t updator, float max_speed) {
    updator->m_max_speed = max_speed;
}

void ui_sprite_ctrl_turntable_updator_stop(ui_sprite_ctrl_turntable_updator_t updator, ui_sprite_ctrl_turntable_member_t member) {
    if (updator->m_curent_op_id) {
        if (member && member->m_turntable) ui_sprite_ctrl_turntable_stop_op(member->m_turntable, updator->m_curent_op_id);
        updator->m_curent_op_id = 0;
    }

    updator->m_duration = 0.0f;
}

static void ui_sprite_ctrl_turntable_updator_update_data(ui_sprite_ctrl_turntable_updator_t updator, ui_sprite_ctrl_turntable_member_t member) {
    updator->m_origin_angle = member->m_angle;
    updator->m_duration = (float)fabs(updator->m_target_angle - updator->m_origin_angle) / updator->m_max_speed;
    updator->m_runing_time = 0.0f;
}

void ui_sprite_ctrl_turntable_updator_set_angle(
    ui_sprite_ctrl_turntable_updator_t updator, ui_sprite_ctrl_turntable_member_t member, float angle)
{
    ui_sprite_ctrl_turntable_t turntable;

    assert(member);

    turntable = member->m_turntable;
    assert(turntable);

    if (updator->m_max_speed == 0.0f) {
        assert(updator->m_curent_op_id == 0);

        updator->m_curent_op_id = ui_sprite_ctrl_turntable_start_op(turntable);
        ui_sprite_ctrl_turntable_update_members_angle(turntable, member, angle);
        ui_sprite_ctrl_turntable_update_members_transform(turntable);
        ui_sprite_ctrl_turntable_stop_op(turntable, updator->m_curent_op_id);
        updator->m_curent_op_id = 0;

        return;
    }

    if (updator->m_duration > 0.0f && fabs(updator->m_target_angle - angle) < 0.01) {
        return;
    }

    updator->m_target_angle = angle;
    ui_sprite_ctrl_turntable_updator_update_data(updator, member);

    if (updator->m_duration == 0.0f) {
        ui_sprite_ctrl_turntable_updator_stop(updator, member);
        return;
    }

    if (updator->m_curent_op_id == 0) {
        updator->m_curent_op_id = ui_sprite_ctrl_turntable_start_op(turntable);
    }

    assert(updator->m_curent_op_id == turntable->m_curent_op_id);
}

void ui_sprite_ctrl_turntable_updator_update(ui_sprite_ctrl_turntable_updator_t updator, ui_sprite_ctrl_turntable_member_t member, float delta) {
    ui_sprite_ctrl_turntable_t turntable;
    float percent;
    float angle;

    assert(member);

    turntable = member->m_turntable;
    assert(turntable);

    if (!ui_sprite_ctrl_turntable_updator_is_runing(updator)) return;

    assert(updator->m_duration > 0.0f);

    if (updator->m_curent_op_id != turntable->m_curent_op_id) {
        if (turntable->m_curent_op_id != 0) return;
        updator->m_curent_op_id = ui_sprite_ctrl_turntable_start_op(turntable);
        ui_sprite_ctrl_turntable_updator_update_data(updator, member);

        if (updator->m_duration == 0.0f) {
            ui_sprite_ctrl_turntable_updator_stop(updator, member);
            return;
        }
    }

    updator->m_runing_time += delta;

    percent = 
        updator->m_runing_time > updator->m_duration
        ? 1.0f
        : (updator->m_runing_time / updator->m_duration);

    percent = ui_percent_decorator_decorate(&updator->m_decorator, percent);

    angle = updator->m_origin_angle + (updator->m_target_angle - updator->m_origin_angle) * percent;

    ui_sprite_ctrl_turntable_update_members_angle(turntable, member, angle);

    if (percent >= 1.0f) ui_sprite_ctrl_turntable_updator_stop(updator, member);
}

uint8_t ui_sprite_ctrl_turntable_updator_is_runing(ui_sprite_ctrl_turntable_updator_t updator) {
    return updator->m_curent_op_id != 0;
}

