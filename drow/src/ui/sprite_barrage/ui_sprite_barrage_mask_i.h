#ifndef UI_SPRITE_BARRAGE_MASK_I_H
#define UI_SPRITE_BARRAGE_MASK_I_H
#include "ui/sprite_barrage/ui_sprite_barrage_mask.h"
#include "ui_sprite_barrage_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_barrage_mask {
    ui_sprite_barrage_module_t m_module;
    char m_new_mask[64];
    char m_emitter_perfix[64];
    uint32_t m_old_mask;
};

int ui_sprite_barrage_mask_regist(ui_sprite_barrage_module_t module);
void ui_sprite_barrage_mask_unregist(ui_sprite_barrage_module_t module);

#ifdef __cplusplus
}
#endif

#endif
