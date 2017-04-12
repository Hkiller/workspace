#ifndef UI_SPRITE_CONTROL_TRACK_META_H
#define UI_SPRITE_CONTROL_TRACK_META_H
#include "ui_sprite_ctrl_types.h"

#ifdef __cplusplus
extern "C" {
#endif

ui_sprite_ctrl_track_meta_t
ui_sprite_ctrl_track_meta_create(
    ui_sprite_ctrl_track_mgr_t mgr, const char * name, const char * anim_layer);
void ui_sprite_ctrl_track_meta_free(ui_sprite_ctrl_track_meta_t track_meta);
ui_sprite_ctrl_track_meta_t ui_sprite_ctrl_track_meta_find(ui_sprite_ctrl_track_mgr_t mgr, const char * name);

int ui_sprite_ctrl_track_meta_add_point(ui_sprite_ctrl_track_meta_t track_meta, float interval, const char * res);

#ifdef __cplusplus
}
#endif

#endif
