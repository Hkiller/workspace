#ifndef UI_SPRITE_BASIC_JOIN_GROUP_I_H
#define UI_SPRITE_BASIC_JOIN_GROUP_I_H
#include "ui/sprite_basic/ui_sprite_basic_join_group.h"
#include "ui_sprite_basic_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_basic_join_group {
    ui_sprite_basic_module_t m_module;
    char * m_group_name;
};

int ui_sprite_basic_join_group_regist(ui_sprite_basic_module_t module);
void ui_sprite_basic_join_group_unregist(ui_sprite_basic_module_t module);

#ifdef __cplusplus
}
#endif

#endif
