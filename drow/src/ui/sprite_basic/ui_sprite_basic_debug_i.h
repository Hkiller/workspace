#ifndef UI_SPRITE_BASIC_DEBUG_I_H
#define UI_SPRITE_BASIC_DEBUG_I_H
#include "ui/sprite_basic/ui_sprite_basic_debug.h"
#include "ui_sprite_basic_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_basic_debug {
    uint8_t m_old_debug;
};

int ui_sprite_basic_debug_regist(ui_sprite_basic_module_t module);
void ui_sprite_basic_debug_unregist(ui_sprite_basic_module_t module);

#ifdef __cplusplus
}
#endif

#endif
