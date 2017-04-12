#ifndef UI_SPRITE_2D_FLIP_I_H
#define UI_SPRITE_2D_FLIP_I_H
#include "ui/sprite_2d/ui_sprite_2d_flip.h"
#include "ui_sprite_2d_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif
	
struct ui_sprite_2d_flip {
	ui_sprite_2d_module_t	m_module;
    uint32_t m_flip_entity_id;
    char m_flip_entity_name[64];
    uint8_t m_flip_to_entity_pos;
    uint8_t m_process_x;
    uint8_t m_process_y;
};

int ui_sprite_2d_flip_regist(ui_sprite_2d_module_t module);
void ui_sprite_2d_flip_unregist(ui_sprite_2d_module_t module);

#ifdef __cplusplus
}
#endif

#endif
