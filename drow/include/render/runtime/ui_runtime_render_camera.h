#ifndef UI_RUNTIME_RENDER_CAMER_H
#define UI_RUNTIME_RENDER_CAMER_H
#include "ui_runtime_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*ui_runtime_render_camera_custom_update_fun_t)(void * ctx, ui_runtime_render_camera_t camera, ui_transform_t transform);
    
ui_runtime_render_camera_t ui_runtime_render_camera_create(ui_runtime_render_t render, const char * name);
void ui_runtime_render_camera_free(ui_runtime_render_camera_t camera);

ui_runtime_render_camera_t ui_runtime_render_camera_find(ui_runtime_render_t render, const char * name);

ui_runtime_render_projection_t ui_runtime_render_camera_projection(ui_runtime_render_camera_t camera);
int ui_runtime_render_camera_set_projection(ui_runtime_render_camera_t camera, ui_runtime_render_projection_t projection);
int ui_runtime_render_camera_set_projection_custom(
    ui_runtime_render_camera_t camera,
    ui_runtime_render_camera_custom_update_fun_t update_fun, void * update_ctx);

void ui_runtime_render_camera_update(ui_runtime_render_camera_t camera);
    
#ifdef __cplusplus
}
#endif

#endif
