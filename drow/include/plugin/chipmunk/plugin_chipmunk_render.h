#ifndef DROW_PLUGIN_CHIPMUNK_RENDER_OBJ_H
#define DROW_PLUGIN_CHIPMUNK_RENDER_OBJ_H
#include "plugin_chipmunk_types.h"

#ifdef __cplusplus
extern "C" {
#endif

void plugin_chipmunk_render_set_env(plugin_chipmunk_render_t render, plugin_chipmunk_env_t env);

void plugin_chipmunk_render_shape(ui_runtime_render_t context, ui_rect_t clip_rect, ui_transform_t transform, void * shape);

#ifdef __cplusplus
}
#endif

#endif

