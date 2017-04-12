#ifndef UI_SPRITE_CAMERA_UPDATOR_H
#define UI_SPRITE_CAMERA_UPDATOR_H
#include "ui_sprite_camera_env_i.h"
#include "render/utils/ui_percent_decorator.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ui_sprite_camera_updator * ui_sprite_camera_updator_t;

struct ui_sprite_camera_updator {
    uint32_t m_curent_op_id;
    struct ui_percent_decorator m_decorator;
    ui_rect m_origin_rect;
    ui_rect m_target_rect;
    float m_max_speed;
    float m_duration;
    float m_runing_time;
};

void ui_sprite_camera_updator_stop(ui_sprite_camera_updator_t updator, ui_sprite_camera_env_t camera);
void ui_sprite_camera_updator_set_max_speed(ui_sprite_camera_updator_t updator, float max_speed);

void ui_sprite_camera_updator_set_camera(ui_sprite_camera_updator_t updator, ui_sprite_camera_env_t camera, ui_vector_2 pos, ui_vector_2_t scale);
void ui_sprite_camera_updator_update(ui_sprite_camera_updator_t updator, ui_sprite_camera_env_t camera, float delta);
uint8_t ui_sprite_camera_updator_is_runing(ui_sprite_camera_updator_t updator);

#ifdef __cplusplus
}
#endif

#endif
