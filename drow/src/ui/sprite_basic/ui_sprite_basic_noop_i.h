#ifndef UI_SPRITE_BASIC_NOOP_I_H
#define UI_SPRITE_BASIC_NOOP_I_H
#include "ui/sprite_basic/ui_sprite_basic_noop.h"
#include "ui_sprite_basic_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

int ui_sprite_basic_noop_regist(ui_sprite_basic_module_t module);
void ui_sprite_basic_noop_unregist(ui_sprite_basic_module_t module);

#ifdef __cplusplus
}
#endif

#endif
