#ifndef UI_SPRITE_CHIPMUNK_TRACK_ANGLE_I_H
#define UI_SPRITE_CHIPMUNK_TRACK_ANGLE_I_H
#include "ui/sprite_chipmunk/ui_sprite_chipmunk_track_angle.h"
#include "ui_sprite_chipmunk_module_i.h"
#include "ui_sprite_chipmunk_obj_updator_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_chipmunk_track_angle {
    ui_sprite_chipmunk_module_t m_module;
    ui_sprite_chipmunk_obj_updator_t m_updator;
};

int ui_sprite_chipmunk_track_angle_regist(ui_sprite_chipmunk_module_t module);
void ui_sprite_chipmunk_track_angle_unregist(ui_sprite_chipmunk_module_t module);
    
#ifdef __cplusplus
}
#endif

#endif
