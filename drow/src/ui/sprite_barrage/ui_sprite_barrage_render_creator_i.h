#ifndef UI_SPRITE_BARRAGE_RENDER_CREATOR_I_H
#define UI_SPRITE_BARRAGE_RENDER_CREATOR_I_H
#include "ui_sprite_barrage_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

int ui_sprite_barrage_render_creator_regist(ui_sprite_barrage_module_t module);
void ui_sprite_barrage_render_creator_unregist(ui_sprite_barrage_module_t module);
    
#ifdef __cplusplus
}
#endif

#endif
