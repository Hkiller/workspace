#ifndef UI_SPRITE_CTRL_TRACK_FOLLOW_I_H
#define UI_SPRITE_CTRL_TRACK_FOLLOW_I_H
#include "ui/sprite_2d/ui_sprite_2d_types.h"
#include "ui/sprite_ctrl/ui_sprite_ctrl_track_follow.h"
#include "ui_sprite_ctrl_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif
	
struct ui_sprite_ctrl_track_follow {
	ui_sprite_ctrl_module_t m_module;

    uint8_t m_track_pos;
    char m_track_name[64];
};

int ui_sprite_ctrl_track_follow_regist(ui_sprite_ctrl_module_t module);
void ui_sprite_ctrl_track_follow_unregist(ui_sprite_ctrl_module_t module);

#ifdef __cplusplus
}
#endif

#endif
