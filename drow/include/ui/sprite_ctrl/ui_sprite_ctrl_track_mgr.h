#ifndef UI_SPRITE_CONTROL_TRACK_MGR_H
#define UI_SPRITE_CONTROL_TRACK_MGR_H
#include "ui_sprite_ctrl_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*track_mgr*/
extern const char * UI_SPRITE_CTRL_TRACK_MGR_TYPE_NAME;

ui_sprite_ctrl_track_mgr_t ui_sprite_ctrl_track_mgr_create(ui_sprite_ctrl_module_t module, ui_sprite_world_t world);
void ui_sprite_ctrl_track_mgr_free(ui_sprite_ctrl_track_mgr_t track_mgr);
ui_sprite_ctrl_track_mgr_t ui_sprite_ctrl_track_mgr_find(ui_sprite_world_t world);

#ifdef __cplusplus
}
#endif

#endif
