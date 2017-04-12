#ifndef UI_SPRITE_CONTROL_TRACK_H
#define UI_SPRITE_CONTROL_TRACK_H
#include "render/utils/ui_vector_2.h"
#include "ui_sprite_ctrl_types.h"

#ifdef __cplusplus
extern "C" {
#endif

ui_sprite_ctrl_track_t ui_sprite_ctrl_track_create(
    ui_sprite_ctrl_track_mgr_t track_mgr, const char * name, const char * type_name);
void ui_sprite_ctrl_track_free(ui_sprite_ctrl_track_t track);
ui_sprite_ctrl_track_t ui_sprite_ctrl_track_find(ui_sprite_ctrl_track_mgr_t track_mgr, const char * name);

int ui_sprite_ctrl_track_show(ui_sprite_ctrl_track_t track);
uint8_t ui_sprite_ctrl_track_is_show(ui_sprite_ctrl_track_t track);
void ui_sprite_ctrl_track_hide(ui_sprite_ctrl_track_t track);

int ui_sprite_ctrl_track_add_point(ui_sprite_ctrl_track_t track, ui_vector_2 pt);

#ifdef __cplusplus
}
#endif

#endif
