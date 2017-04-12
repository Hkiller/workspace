#ifndef UI_SPRITE_2D_PART_I_H
#define UI_SPRITE_2D_PART_I_H
#include "render/utils/ui_transform.h"
#include "ui/sprite_2d/ui_sprite_2d_part.h"
#include "ui_sprite_2d_transform_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_2d_part {
    ui_sprite_2d_transform_t m_transform;
    TAILQ_ENTRY(ui_sprite_2d_part) m_next;
    ui_sprite_2d_part_binding_list_t m_bindings;
    ui_sprite_2d_part_attr_list_t m_attrs;
    uint8_t m_attr_updated;
    uint8_t m_trans_updated;
    char m_name[32];
    ui_transform m_trans;
};

void ui_sprite_2d_part_real_free(
    ui_sprite_2d_module_t module, ui_sprite_2d_part_t emitter);

#ifdef __cplusplus
}
#endif

#endif
