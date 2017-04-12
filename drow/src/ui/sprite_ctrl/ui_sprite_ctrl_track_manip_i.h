#ifndef UI_SPRITE_CTRL_TRACK_MANIP_I_H
#define UI_SPRITE_CTRL_TRACK_MANIP_I_H
#include "ui/sprite_ctrl/ui_sprite_ctrl_track_manip.h"
#include "ui_sprite_ctrl_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif
	
struct ui_sprite_ctrl_track_manip {
	ui_sprite_ctrl_module_t m_module;

    uint8_t m_track_pos;
    ui_vector_2 m_pre_pos;
};

int ui_sprite_ctrl_track_manip_regist(ui_sprite_ctrl_module_t module);
void ui_sprite_ctrl_track_manip_unregist(ui_sprite_ctrl_module_t module);

#ifdef __cplusplus
}
#endif

#endif
