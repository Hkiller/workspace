#ifndef DROW_PLUGIN_UI_ANIM_PROGRESS_BIND_CONTROL_H
#define DROW_PLUGIN_UI_ANIM_PROGRESS_BIND_CONTROL_H
#include "plugin_ui_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * PLUGIN_UI_ANIM_PROGRESS_BIND_CONTROL_NAME;

plugin_ui_anim_progress_bind_control_t plugin_ui_anim_progress_bind_control_create(plugin_ui_env_t env);
int plugin_ui_anim_progress_bind_control_add_frame(plugin_ui_anim_progress_bind_control_t anim_resize, plugin_ui_control_frame_t frame);

#ifdef __cplusplus
}
#endif

#endif
