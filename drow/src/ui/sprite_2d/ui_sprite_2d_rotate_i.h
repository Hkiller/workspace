#ifndef UI_SPRITE_2D_ROTATE_TO_I_H
#define UI_SPRITE_2D_ROTATE_TO_I_H
#include "render/utils/ui_percent_decorator.h"
#include "ui/sprite_2d/ui_sprite_2d_rotate.h"
#include "ui_sprite_2d_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_2d_rotate {
    ui_sprite_2d_module_t m_module;
    char * m_cfg_op;
    char * m_cfg_speed;    
    struct ui_percent_decorator m_cfg_decorator;

    float m_target_angle;
    float m_origin_angle;
    float m_runing_time;
    float m_duration;
};

int ui_sprite_2d_rotate_regist(ui_sprite_2d_module_t module);
void ui_sprite_2d_rotate_unregist(ui_sprite_2d_module_t module);

#ifdef __cplusplus
}
#endif

#endif
