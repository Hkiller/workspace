#ifndef UI_SPRITE_BASIC_CHECK_ENTITY_EXIST_I_H
#define UI_SPRITE_BASIC_CHECK_ENTITY_EXIST_I_H
#include "ui/sprite_basic/ui_sprite_basic_wait_entity_destory.h"
#include "ui_sprite_basic_module_i.h"
#include "ui_sprite_basic_value_generator_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_basic_wait_entity_destory {
	ui_sprite_basic_module_t m_module;
	char * m_cfg_entity_id;

    uint32_t m_entity_id;
};

int ui_sprite_basic_wait_entity_destory_regist(ui_sprite_basic_module_t module);
void ui_sprite_basic_wait_entity_destory_unregist(ui_sprite_basic_module_t module);

#ifdef __cplusplus
}
#endif

#endif
