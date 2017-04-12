#ifndef DROW_PLUGIN_UI_ANIMATION_FRAME_BIND_H
#define DROW_PLUGIN_UI_ANIMATION_FRAME_BIND_H
#include "plugin_ui_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * PLUGIN_UI_ANIM_FRAME_BIND;
    
plugin_ui_anim_frame_bind_t
plugin_ui_anim_frame_bind_create(plugin_ui_control_frame_t frame, char * arg_buf_will_change);

int plugin_ui_anim_frame_bind_set_target(plugin_ui_anim_frame_bind_t frame_bind, const char * target);

#ifdef __cplusplus
}
#endif

#endif

