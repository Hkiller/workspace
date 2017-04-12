#ifndef DROW_PLUGIN_UI_ANIMATION_FRAME_MOVE_H
#define DROW_PLUGIN_UI_ANIMATION_FRAME_MOVE_H
#include "plugin_ui_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * PLUGIN_UI_ANIM_FRAME_MOVE;
    
plugin_ui_anim_frame_move_t
plugin_ui_anim_frame_move_create(plugin_ui_control_frame_t frame, char * arg_buf_will_change);

#ifdef __cplusplus
}
#endif

#endif

