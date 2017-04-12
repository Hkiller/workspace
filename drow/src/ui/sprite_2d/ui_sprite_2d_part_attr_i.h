#ifndef UI_SPRITE_2D_PART_ATTR_I_H
#define UI_SPRITE_2D_PART_ATTR_I_H
#include "ui/sprite_2d/ui_sprite_2d_part_attr.h"
#include "ui_sprite_2d_part_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_2d_part_attr {
    ui_sprite_2d_part_t m_part;
    TAILQ_ENTRY(ui_sprite_2d_part_attr) m_next;
    char m_name[32];
    char m_inline_value[32];
    char * m_value;
    uint8_t m_value_changed;
};

void ui_sprite_2d_part_attr_real_free(
    ui_sprite_2d_module_t module, ui_sprite_2d_part_attr_t attr);

#ifdef __cplusplus
}
#endif

#endif
