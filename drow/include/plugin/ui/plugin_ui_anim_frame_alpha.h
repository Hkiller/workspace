#ifndef DROW_PLUGIN_UI_ANIMATION_FRAME_ALPHA_H
#define DROW_PLUGIN_UI_ANIMATION_FRAME_ALPHA_H
#include "plugin_ui_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * PLUGIN_UI_ANIM_FRAME_ALPHA;
    
plugin_ui_anim_frame_alpha_t
plugin_ui_anim_frame_alpha_create(plugin_ui_control_frame_t frame, char * arg_buf_will_change);

#ifdef __cplusplus
}
#endif

#endif

