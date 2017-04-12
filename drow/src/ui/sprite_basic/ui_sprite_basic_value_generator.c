#include <assert.h>
#include "cpe/pal/pal_stdio.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite/ui_sprite_entity_attr.h"
#include "ui/sprite/ui_sprite_entity_calc.h"
#include "ui_sprite_basic_value_generator_i.h"

ui_sprite_basic_value_generator_t 
ui_sprite_basic_value_generator_create(
    ui_sprite_basic_module_t module, ui_sprite_basic_value_generator_list_t * mgr,
    UI_SPRITE_BASIC_VALUE_GENEARTOR_DEF const * def)
{
    ui_sprite_basic_value_generator_t  generator;

    generator = mem_alloc(module->m_alloc, sizeof(struct ui_sprite_basic_value_generator));
    if (generator == NULL) {
        CPE_ERROR(module->m_em, "ui_sprite_basic_value_generator_create: create fail");
        return NULL;
    }

    generator->m_module = module;
    generator->m_def = *def;

    TAILQ_INIT(&generator->m_values);
    TAILQ_INIT(&generator->m_free_values);

    TAILQ_INSERT_TAIL(mgr, generator, m_next_for_mgr);

    return generator;
}

ui_sprite_basic_value_generator_t 
ui_sprite_basic_value_generator_clone(
    ui_sprite_basic_module_t module,
    ui_sprite_basic_value_generator_list_t * mgr,
    ui_sprite_basic_value_generator_t  from_generator)
{
    ui_sprite_basic_value_generator_t  generator;

    generator = ui_sprite_basic_value_generator_create(module, mgr, &from_generator->m_def);

    return generator;
}

int ui_sprite_basic_value_generator_clone_all(
    ui_sprite_basic_module_t module,
    ui_sprite_basic_value_generator_list_t * mgr,
    ui_sprite_basic_value_generator_list_t * from_mgr)
{
    ui_sprite_basic_value_generator_t from_generator;

    TAILQ_FOREACH(from_generator, from_mgr, m_next_for_mgr) {
        if (ui_sprite_basic_value_generator_clone(module, mgr, from_generator) == NULL) return -1;
    }

    return 0;
}

void ui_sprite_basic_value_generator_free(ui_sprite_basic_value_generator_t generator, ui_sprite_basic_value_generator_list_t * mgr) {
    ui_sprite_basic_module_t module = generator->m_module;

    while(!TAILQ_EMPTY(&generator->m_values)) {
        ui_sprite_basic_value_generator_value_t value = TAILQ_FIRST(&generator->m_values);
        TAILQ_REMOVE(&generator->m_values, value, m_next);
        mem_free(module->m_alloc, value);
    }

    while(!TAILQ_EMPTY(&generator->m_free_values)) {
        ui_sprite_basic_value_generator_value_t value = TAILQ_FIRST(&generator->m_free_values);
        TAILQ_REMOVE(&generator->m_values, value, m_next);
        mem_free(module->m_alloc, value);
    }

    TAILQ_REMOVE(mgr, generator, m_next_for_mgr);

    mem_free(module->m_alloc, generator);
}

void ui_sprite_basic_value_generator_free_all(ui_sprite_basic_value_generator_list_t * mgr) {
    while(!TAILQ_EMPTY(mgr)) {
        ui_sprite_basic_value_generator_free(TAILQ_FIRST(mgr), mgr);
    }
}

extern int ui_sprite_basic_value_generator_init_in_range(
    ui_sprite_basic_value_generator_t generator, uint16_t once_gen_count, ui_sprite_entity_t entity, dr_data_source_t data_source);
extern int ui_sprite_basic_value_generator_init_rand(
    ui_sprite_basic_value_generator_t generator, uint16_t once_gen_count, ui_sprite_entity_t entity, dr_data_source_t data_source);

int ui_sprite_basic_value_generator_init(ui_sprite_basic_value_generator_t generator, ui_sprite_entity_t entity, dr_data_source_t data_source) {
    int64_t once_gen_count;

    while(!TAILQ_EMPTY(&generator->m_values)) {
        ui_sprite_basic_value_generator_value_t value = TAILQ_FIRST(&generator->m_values);
        TAILQ_REMOVE(&generator->m_values, value, m_next);
        TAILQ_INSERT_TAIL(&generator->m_free_values, value, m_next);
    }

    if (generator->m_def.once_gen_count[0]) {
        if (ui_sprite_entity_try_calc_int64(
                &once_gen_count, generator->m_def.once_gen_count, entity, data_source, generator->m_module->m_em)
            != 0)
        {
            CPE_ERROR(
                generator->m_module->m_em, "entity %d(%s): value_generator_init: calc generate count %s fail!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), generator->m_def.once_gen_count);
            return -1;
        }

        if (once_gen_count < 1) {
            CPE_ERROR(
                generator->m_module->m_em, "entity %d(%s): value_generator_init: once gen count "FMT_INT64_T" calc from '%s' error!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), once_gen_count, generator->m_def.once_gen_count);
            return -1;
        }
    }
    else {
        once_gen_count = 1;
    }

    switch(generator->m_def.type) {
    case UI_SPRITE_BASIC_VALUE_GENEARTOR_TYPE_IN_RANGE:
        return ui_sprite_basic_value_generator_init_in_range(generator, once_gen_count, entity, data_source);
    case UI_SPRITE_BASIC_VALUE_GENEARTOR_TYPE_RAND:
        return ui_sprite_basic_value_generator_init_rand(generator, once_gen_count, entity, data_source);
    default:
        CPE_ERROR(
            generator->m_module->m_em, "entity %d(%s): value_generator_init: unknown type %d!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), generator->m_def.type);
        return -1;
    }
}

int ui_sprite_basic_value_generator_init_all(ui_sprite_basic_value_generator_list_t * mgr, ui_sprite_entity_t entity, dr_data_source_t data_source) {
    ui_sprite_basic_value_generator_t generator;

    TAILQ_FOREACH(generator, mgr, m_next_for_mgr) {
        if (ui_sprite_basic_value_generator_init(generator, entity, data_source) != 0) return -1;
    }

    return 0;
}

int ui_sprite_basic_value_generator_supply(ui_sprite_basic_value_generator_t generator, ui_sprite_entity_t entity, dr_data_source_t data_source) {
    uint8_t i;

    for(i = 0; i < generator->m_def.supply_count; ++i) {
        ui_sprite_basic_value_generator_value_t value;

        if (TAILQ_EMPTY(&generator->m_values)) {
            if (ui_sprite_basic_value_generator_init(generator, entity, data_source) != 0) return -1;

            if (TAILQ_EMPTY(&generator->m_values)) {
                CPE_ERROR(
                    generator->m_module->m_em, "entity %d(%s): value_generator_supply: after init, no value!",
                    ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
                return -1;
            }
        }

        value = TAILQ_FIRST(&generator->m_values);
        assert(value);

        TAILQ_REMOVE(&generator->m_values, value, m_next);
        TAILQ_INSERT_TAIL(&generator->m_free_values, value, m_next);

        if (ui_sprite_entity_set_attr_value(entity, generator->m_def.supplies[i], &value->m_value) != 0) return -1;
    }

    return 0;
}

int ui_sprite_basic_value_generator_supply_all(ui_sprite_basic_value_generator_list_t * mgr, ui_sprite_entity_t entity, dr_data_source_t data_source) {
    ui_sprite_basic_value_generator_t generator;

    TAILQ_FOREACH(generator, mgr, m_next_for_mgr) {
        if (ui_sprite_basic_value_generator_supply(generator, entity, data_source) != 0) return -1;
    }

    return 0;
}

ui_sprite_basic_value_generator_value_t
ui_sprite_basic_value_generator_value_add(ui_sprite_basic_value_generator_t generator, size_t capacity) {
    ui_sprite_basic_module_t module = generator->m_module;
    ui_sprite_basic_value_generator_value_t r;

    TAILQ_FOREACH(r, &generator->m_free_values, m_next) {
        if (capacity <= r->m_capacity) {
            TAILQ_REMOVE(&generator->m_free_values, r, m_next);
            TAILQ_INSERT_TAIL(&generator->m_values, r, m_next);
            return r;
        }
    }

    r = mem_alloc(module->m_alloc, sizeof(struct ui_sprite_basic_value_generator_value) + capacity);
    if (r == NULL) {
        CPE_ERROR(module->m_em, "ui_sprite_basic_value_generator_value_get: alloc fail!");
        return NULL;
    }

    r->m_capacity = capacity;
    TAILQ_INSERT_TAIL(&generator->m_values, r, m_next);

    return r;
}

int ui_sprite_basic_value_generator_add_float(ui_sprite_basic_value_generator_t generator, float value) {
    ui_sprite_basic_value_generator_value_t r = ui_sprite_basic_value_generator_value_add(generator, sizeof(value));
    if (r == NULL) return -1;
    
    r->m_value.m_type = CPE_DR_TYPE_FLOAT;
    r->m_value.m_meta = NULL;
    r->m_value.m_data = r + 1;
    r->m_value.m_size = sizeof(value);

    memcpy(r->m_value.m_data, &value, sizeof(value));

    return 0;
}
