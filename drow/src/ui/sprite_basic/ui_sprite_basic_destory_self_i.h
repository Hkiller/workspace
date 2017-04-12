#ifndef UI_SPRITE_BASIC_DESTORY_SELF_I_H
#define UI_SPRITE_BASIC_DESTORY_SELF_I_H
#include "ui/sprite_basic/ui_sprite_basic_destory_self.h"
#include "ui_sprite_basic_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_basic_destory_self {
	ui_sprite_basic_module_t m_module;
};

int ui_sprite_basic_destory_self_regist(ui_sprite_basic_module_t module);
void ui_sprite_basic_destory_self_unregist(ui_sprite_basic_module_t module);

#ifdef __cplusplus
}
#endif

#endif
