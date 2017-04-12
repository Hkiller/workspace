#ifndef UI_SPRITE_BASIC_GEN_ENTITIES_I_H
#define UI_SPRITE_BASIC_GEN_ENTITIES_I_H
#include "ui/sprite_basic/ui_sprite_basic_gen_entities.h"
#include "ui_sprite_basic_module_i.h"
#include "ui_sprite_basic_value_generator_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_basic_gen_entities {
    ui_sprite_basic_module_t m_module;
    char m_proto[64];
    uint16_t m_gen_count;
    float m_gen_duration;
    uint8_t m_wait_stop;
    uint8_t m_do_destory;

    char * m_attrs;
    ui_sprite_basic_value_generator_list_t m_generators;

    uint16_t m_generated_count;
    float m_generated_duration;

    uint16_t m_runing_entity_count;
    uint16_t m_runing_entity_capacity;
    uint16_t * m_runing_entities;
};

int ui_sprite_basic_gen_entities_regist(ui_sprite_basic_module_t module);
void ui_sprite_basic_gen_entities_unregist(ui_sprite_basic_module_t module);
ui_sprite_fsm_action_t ui_sprite_basic_gen_entities_load(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg);

#ifdef __cplusplus
}
#endif

#endif
