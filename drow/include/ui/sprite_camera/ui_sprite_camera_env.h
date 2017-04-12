#ifndef UI_SPRITE_CAMERA_ENV_H
#define UI_SPRITE_CAMERA_ENV_H
#include "render/utils/ui_vector_2.h"
#include "ui_sprite_camera_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_CAMERA_ENV_NAME;

ui_sprite_camera_env_t ui_sprite_camera_env_create(ui_sprite_camera_module_t module, ui_sprite_world_t world);
void ui_sprite_camera_env_free(ui_sprite_camera_env_t camera);
ui_sprite_camera_env_t ui_sprite_camera_env_find(ui_sprite_world_t world);

/*镜头范围限制的设定 */
ui_vector_2 ui_sprite_camera_env_limit_lt(ui_sprite_camera_env_t camera);
ui_vector_2 ui_sprite_camera_env_limit_rb(ui_sprite_camera_env_t camera);
int ui_sprite_camera_env_set_limit(ui_sprite_camera_env_t camera, ui_vector_2 lt, ui_vector_2 br);
int8_t ui_sprite_camera_env_have_limit(ui_sprite_camera_env_t camera);

/* 镜头缩放限定 */
int ui_sprite_camera_env_set_scale_range(ui_sprite_camera_env_t camera, float scale_min, float scale_max);
float ui_sprite_camera_env_scale_min(ui_sprite_camera_env_t camera);
float ui_sprite_camera_env_scale_max(ui_sprite_camera_env_t camera);

/*镜头轨道 */
enum ui_sprite_camera_trace_type {
    ui_sprite_camera_trace_none,
    ui_sprite_camera_trace_by_x,
    ui_sprite_camera_trace_by_y
};

int ui_sprite_camera_env_set_trace(
    ui_sprite_camera_env_t camera, enum ui_sprite_camera_trace_type type,
    ui_vector_2 screen_pos, ui_vector_2 world_pos_a, ui_vector_2 world_pos_b);

void ui_sprite_camera_env_remove_trace(ui_sprite_camera_env_t camera);

#ifdef __cplusplus
}
#endif

#endif
