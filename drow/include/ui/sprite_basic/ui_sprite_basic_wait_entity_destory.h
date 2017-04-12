#ifndef UI_SPRITE_BASIC_WAIT_ENTITY_DESTORY_H
#define UI_SPRITE_BASIC_WAIT_ENTITY_DESTORY_H
#include "ui_sprite_basic_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_BASIC_WAIT_ENTITY_DESTORY_NAME;

ui_sprite_basic_wait_entity_destory_t ui_sprite_basic_wait_entity_destory_create(ui_sprite_fsm_state_t fsm_state, const char * name);
void ui_sprite_basic_wait_entity_destory_free(ui_sprite_basic_wait_entity_destory_t wait_entity_destory);

const char * ui_sprite_basic_wait_entity_destory_entity_id(ui_sprite_basic_wait_entity_destory_t entity_exitst);
int ui_sprite_basic_wait_entity_destory_set_entity_id(ui_sprite_basic_wait_entity_destory_t entity_exitst, const char * entity_id);

#ifdef __cplusplus
}
#endif

#endif
