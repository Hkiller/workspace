#ifndef UI_SPRITE_RENDER_LOCK_ON_SCREEN_I_H
#define UI_SPRITE_RENDER_LOCK_ON_SCREEN_I_H
#include "render/utils/ui_percent_decorator.h"
#include "ui/sprite_render/ui_sprite_render_lock_on_screen.h"
#include "ui_sprite_render_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_render_lock_on_screen {
    ui_sprite_render_module_t m_module;
    char * m_cfg_x;
    char * m_cfg_y;
    struct ui_percent_decorator m_decorator;
    float m_max_speed;

    ui_vector_2 m_target_pos_on_screen;
    ui_vector_2 m_init_pos_on_screen;
    float m_runing_time;
    float m_duration;
};

int ui_sprite_render_lock_on_screen_regist(ui_sprite_render_module_t module);
void ui_sprite_render_lock_on_screen_unregist(ui_sprite_render_module_t module);

#ifdef __cplusplus
}
#endif

#endif
