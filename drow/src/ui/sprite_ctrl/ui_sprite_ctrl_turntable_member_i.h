#ifndef UI_SPRITE_CTRL_TURNTABLE_MEMBER_I_H
#define UI_SPRITE_CTRL_TURNTABLE_MEMBER_I_H
#include "cpe/pal/pal_queue.h"
#include "ui/sprite_ctrl/ui_sprite_ctrl_turntable_member.h"
#include "ui_sprite_ctrl_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef TAILQ_HEAD(ui_sprite_ctrl_turntable_member_list, ui_sprite_ctrl_turntable_member) ui_sprite_ctrl_turntable_member_list_t;

struct ui_sprite_ctrl_turntable_member {
    ui_sprite_ctrl_module_t m_module;
    ui_sprite_ctrl_turntable_t m_turntable;
    TAILQ_ENTRY(ui_sprite_ctrl_turntable_member) m_next_for_turntable;
    char * m_on_select;
    char * m_on_unselect;
    float m_angle;
};

int ui_sprite_ctrl_turntable_member_regist(ui_sprite_ctrl_module_t module);
void ui_sprite_ctrl_turntable_member_unregist(ui_sprite_ctrl_module_t module);

#ifdef __cplusplus
}
#endif

#endif
