#ifndef UI_SPRITE_BASIC_VALUE_GENERATOR_I_H
#define UI_SPRITE_BASIC_VALUE_GENERATOR_I_H
#include "ui/sprite_basic/ui_sprite_basic_value_generator.h"
#include "ui_sprite_basic_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef TAILQ_HEAD(ui_sprite_basic_value_generator_list, ui_sprite_basic_value_generator) ui_sprite_basic_value_generator_list_t;

typedef struct ui_sprite_basic_value_generator_value * ui_sprite_basic_value_generator_value_t;
typedef TAILQ_HEAD(ui_sprite_basic_value_generator_value_list, ui_sprite_basic_value_generator_value) ui_sprite_basic_value_generator_value_list_t;

struct ui_sprite_basic_value_generator_value {
    TAILQ_ENTRY(ui_sprite_basic_value_generator_value) m_next;
    uint16_t m_capacity;
    struct dr_value m_value;
};

struct ui_sprite_basic_value_generator {
    ui_sprite_basic_module_t m_module;
    TAILQ_ENTRY(ui_sprite_basic_value_generator) m_next_for_mgr;
    UI_SPRITE_BASIC_VALUE_GENEARTOR_DEF m_def;
    ui_sprite_basic_value_generator_value_list_t m_values;
    ui_sprite_basic_value_generator_value_list_t m_free_values;
};

ui_sprite_basic_value_generator_t 
ui_sprite_basic_value_generator_create(
    ui_sprite_basic_module_t module,
    ui_sprite_basic_value_generator_list_t * mgr,
    UI_SPRITE_BASIC_VALUE_GENEARTOR_DEF const * def);

ui_sprite_basic_value_generator_t 
ui_sprite_basic_value_generator_clone(
    ui_sprite_basic_module_t module,
    ui_sprite_basic_value_generator_list_t * mgr,
    ui_sprite_basic_value_generator_t  from_generator);

int ui_sprite_basic_value_generator_clone_all(
    ui_sprite_basic_module_t module,
    ui_sprite_basic_value_generator_list_t * mgr,
    ui_sprite_basic_value_generator_list_t * from_mgr);

void ui_sprite_basic_value_generator_free(ui_sprite_basic_value_generator_t generator, ui_sprite_basic_value_generator_list_t * mgr);
void ui_sprite_basic_value_generator_free_all(ui_sprite_basic_value_generator_list_t * mgr);

int ui_sprite_basic_value_generator_init(ui_sprite_basic_value_generator_t generator, ui_sprite_entity_t entity, dr_data_source_t data_source);
int ui_sprite_basic_value_generator_init_all(ui_sprite_basic_value_generator_list_t * mgr, ui_sprite_entity_t entity, dr_data_source_t data_source);

int ui_sprite_basic_value_generator_supply(ui_sprite_basic_value_generator_t generator, ui_sprite_entity_t entity, dr_data_source_t data_source);
int ui_sprite_basic_value_generator_supply_all(ui_sprite_basic_value_generator_list_t * mgr, ui_sprite_entity_t entity, dr_data_source_t data_source);

int ui_sprite_basic_value_generator_add_float(ui_sprite_basic_value_generator_t generator, float value);

int ui_sprite_basic_value_generator_load(ui_sprite_basic_module_t module, UI_SPRITE_BASIC_VALUE_GENEARTOR_DEF * def, cfg_t cfg);
    
#ifdef __cplusplus
}
#endif

#endif
