#ifndef UI_SPRITE_BARRAGE_PAUSE_EMITTER_I_H
#define UI_SPRITE_BARRAGE_PAUSE_EMITTER_I_H
#include "ui/sprite_barrage/ui_sprite_barrage_pause_emitter.h"
#include "ui_sprite_barrage_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_barrage_pause_emitter {
    ui_sprite_barrage_module_t m_module;
    char m_group_name[64];
};

int ui_sprite_barrage_pause_emitter_regist(ui_sprite_barrage_module_t module);
void ui_sprite_barrage_pause_emitter_unregist(ui_sprite_barrage_module_t module);

#ifdef __cplusplus
}
#endif

#endif
