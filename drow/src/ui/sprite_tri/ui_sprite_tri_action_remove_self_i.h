#ifndef UI_SPRITE_TRI_ACTION_REMOVE_SELF_I_H
#define UI_SPRITE_TRI_ACTION_REMOVE_SELF_I_H
#include "ui/sprite_tri/ui_sprite_tri_action_remove_self.h"
#include "ui_sprite_tri_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_tri_action_remove_self {
    ui_sprite_tri_module_t m_module;
};

int ui_sprite_tri_action_remove_self_regist(ui_sprite_tri_module_t module);
void ui_sprite_tri_action_remove_self_unregist(ui_sprite_tri_module_t module);

#ifdef __cplusplus
}
#endif

#endif
