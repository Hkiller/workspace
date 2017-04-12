#ifndef UI_SPRITE_2D_ACTION_PART_FOLLOW_I_H
#define UI_SPRITE_2D_ACTION_PART_FOLLOW_I_H
#include "ui/sprite_2d/ui_sprite_2d_action_part_follow.h"
#include "ui_sprite_2d_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif
	
struct ui_sprite_2d_action_part_follow {
	ui_sprite_2d_module_t m_module;
    char * m_cfg_part;
    char * m_cfg_target;

    ui_sprite_2d_part_t m_part;
    char * m_target_entity;
    char * m_target_part;
};

int ui_sprite_2d_action_part_follow_regist(ui_sprite_2d_module_t module);
void ui_sprite_2d_action_part_follow_unregist(ui_sprite_2d_module_t module);

#ifdef __cplusplus
}
#endif

#endif
