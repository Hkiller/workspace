#ifndef UI_SPRITE_BASIC_WAIT_CONDITION_I_H
#define UI_SPRITE_BASIC_WAIT_CONDITION_I_H
#include "ui/sprite_basic/ui_sprite_basic_wait_condition.h"
#include "ui_sprite_basic_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_basic_wait_condition {
    ui_sprite_basic_module_t m_module;
    char * m_cfg_check;
};

int ui_sprite_basic_wait_condition_regist(ui_sprite_basic_module_t module);
void ui_sprite_basic_wait_condition_unregist(ui_sprite_basic_module_t module);

#ifdef __cplusplus
}
#endif

#endif
