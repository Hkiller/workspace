#ifndef UI_SPRITE_SCROLLMAP_SET_SPEED_I_H
#define UI_SPRITE_SCROLLMAP_SET_SPEED_I_H
#include "ui/sprite_scrollmap/ui_sprite_scrollmap_set_speed.h"
#include "ui_sprite_scrollmap_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_scrollmap_set_speed {
    ui_sprite_scrollmap_module_t m_module;
    float m_to_speed;
    float m_acceleration;
};

int ui_sprite_scrollmap_set_speed_regist(ui_sprite_scrollmap_module_t module);
void ui_sprite_scrollmap_set_speed_unregist(ui_sprite_scrollmap_module_t module);

#ifdef __cplusplus
}
#endif

#endif
