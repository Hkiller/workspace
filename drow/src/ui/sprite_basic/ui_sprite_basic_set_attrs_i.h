#ifndef UI_SPRITE_BASIC_SET_ATTR_I_H
#define UI_SPRITE_BASIC_SET_ATTR_I_H
#include "ui/sprite_basic/ui_sprite_basic_set_attrs.h"
#include "ui_sprite_basic_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_basic_set_attrs {
    ui_sprite_basic_module_t m_module;
    char * m_setter;
};

int ui_sprite_basic_set_attrs_regist(ui_sprite_basic_module_t module);
void ui_sprite_basic_set_attrs_unregist(ui_sprite_basic_module_t module);

#ifdef __cplusplus
}
#endif

#endif
