#ifndef UI_SPRITE_CTRL_TURNTABLE_I_H
#define UI_SPRITE_CTRL_TURNTABLE_I_H
#include "ui/sprite_ctrl/ui_sprite_ctrl_turntable.h"
#include "ui_sprite_ctrl_module_i.h"
#include "ui_sprite_ctrl_turntable_member_i.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*ui_sprite_ctrl_turntable_track_fun_t)(
    ui_sprite_ctrl_turntable_t turntable, ui_vector_2_t scale,
    ui_vector_2_t r, ui_vector_2 const * base, float angle);

struct ui_sprite_ctrl_turntable {
    ui_sprite_ctrl_module_t m_module;

    /*config */
    ui_sprite_ctrl_turntable_track_fun_t m_track_fun;
    UI_SPRITE_CTRL_TURNTABLE_DEF m_def;
    uint16_t m_focuse_slot_idx;

    /*data*/
    UI_SPRITE_CTRL_TURNTABLE_DATA m_data;

    /*runtime */
    ui_sprite_ctrl_turntable_member_list_t m_members;
    ui_sprite_ctrl_turntable_member_t m_focuse_member;
    uint32_t m_max_op_id;
    uint32_t m_curent_op_id;
};

int ui_sprite_ctrl_turntable_regist(ui_sprite_ctrl_module_t module);
void ui_sprite_ctrl_turntable_unregist(ui_sprite_ctrl_module_t module);
int ui_sprite_ctrl_turntable_load(void * ctx, ui_sprite_component_t component, cfg_t cfg);

void ui_sprite_ctrl_turntable_update_members_transform(ui_sprite_ctrl_turntable_t turntable);

void ui_sprite_ctrl_turntable_update_members_angle(
    ui_sprite_ctrl_turntable_t turntable, ui_sprite_ctrl_turntable_member_t base_member, float base_angle);

ui_sprite_ctrl_turntable_member_t
ui_sprite_ctrl_turntable_find_focuse_member(ui_sprite_ctrl_turntable_t turntable, float diff_to_angle);

float ui_sprite_ctrl_turntable_calc_member_angle(
    ui_sprite_ctrl_turntable_t turntable, ui_sprite_ctrl_turntable_member_t member,
    ui_sprite_ctrl_turntable_member_t base_member, float base_member_angle);

int ui_sprite_ctrl_turntable_pos(ui_sprite_ctrl_turntable_t turntable, ui_vector_2 * pos);

uint32_t ui_sprite_ctrl_turntable_start_op(ui_sprite_ctrl_turntable_t turntable);
void ui_sprite_ctrl_turntable_stop_op(ui_sprite_ctrl_turntable_t turntable, uint32_t op_id);

#ifdef __cplusplus
}
#endif

#endif
