#ifndef UI_SPRITE_2D_TRANSFORM_I_H
#define UI_SPRITE_2D_TRANSFORM_I_H
#include "ui/sprite_2d/ui_sprite_2d_transform.h"
#include "ui_sprite_2d_module_i.h"
#include "protocol/ui/sprite_2d/ui_sprite_2d_data.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_2d_transform {
    ui_sprite_2d_module_t m_module;
    ui_sprite_2d_part_list_t m_parts;
    UI_SPRITE_2D_TRANSFORM m_data;
};

int ui_sprite_2d_transform_regist(ui_sprite_2d_module_t module);
void ui_sprite_2d_transform_unregist(ui_sprite_2d_module_t module);

#ifdef __cplusplus
}
#endif

#endif
