#ifndef UI_SPRITE_BASIC_CHECK_ENTITY_NOT_IN_STATE_I_H
#define UI_SPRITE_BASIC_CHECK_ENTITY_NOT_IN_STATE_I_H
#include "ui/sprite_basic/ui_sprite_basic_wait_entity_not_in_state.h"
#include "ui_sprite_basic_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_basic_wait_entity_not_in_state {
	ui_sprite_basic_module_t m_module;
	char * m_cfg_entity;
	char * m_cfg_state;

    uint32_t m_group;
    uint32_t m_entity;
    char * m_state;
};

int ui_sprite_basic_wait_entity_not_in_state_regist(ui_sprite_basic_module_t module);
void ui_sprite_basic_wait_entity_not_in_state_unregist(ui_sprite_basic_module_t module);

#ifdef __cplusplus
}
#endif

#endif
