#ifndef UI_SPRITE_BASIC_GEN_ENTITIES_H
#define UI_SPRITE_BASIC_GEN_ENTITIES_H
#include "ui_sprite_basic_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_BASIC_GEN_ENTITIES_NAME;

ui_sprite_basic_gen_entities_t ui_sprite_basic_gen_entities_create(ui_sprite_fsm_state_t fsm_state, const char * name);
void ui_sprite_basic_gen_entities_free(ui_sprite_basic_gen_entities_t gen_eitities);

const char * ui_sprite_basic_gen_entities_proto(ui_sprite_basic_gen_entities_t gen_eitities);
void ui_sprite_basic_gen_entities_set_proto(ui_sprite_basic_gen_entities_t gen_eitities, const char * proto);

uint8_t ui_sprite_basic_gen_entities_wait_stop(ui_sprite_basic_gen_entities_t gen_eitities);
void ui_sprite_basic_gen_entities_set_wait_stop(ui_sprite_basic_gen_entities_t gen_eitities, uint8_t wait_stop);

const char * ui_sprite_basic_gen_entities_attrs(ui_sprite_basic_gen_entities_t gen_eitities);
int ui_sprite_basic_gen_entities_set_attrs(ui_sprite_basic_gen_entities_t gen_eitities, const char * attrs);

uint8_t ui_sprite_basic_gen_entities_do_destory(ui_sprite_basic_gen_entities_t gen_eitities);
void ui_sprite_basic_gen_entities_set_do_destory(ui_sprite_basic_gen_entities_t gen_eitities, uint8_t do_destory);

ui_sprite_basic_value_generator_t 
ui_sprite_basic_gen_entities_create_generator(ui_sprite_basic_gen_entities_t gen_eitities, UI_SPRITE_BASIC_VALUE_GENEARTOR_DEF const * def);

#ifdef __cplusplus
}
#endif

#endif
