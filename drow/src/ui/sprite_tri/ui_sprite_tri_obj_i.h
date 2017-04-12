#ifndef UI_SPRITE_TRI_OBJ_I_H
#define UI_SPRITE_TRI_OBJ_I_H
#include "ui/sprite_tri/ui_sprite_tri_obj.h"
#include "ui_sprite_tri_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_tri_obj {
    ui_sprite_tri_module_t m_module;
    ui_sprite_tri_rule_list_t m_rules;
};

int ui_sprite_tri_obj_regist(ui_sprite_tri_module_t module);
void ui_sprite_tri_obj_unregist(ui_sprite_tri_module_t module);

#ifdef __cplusplus
}
#endif

#endif
