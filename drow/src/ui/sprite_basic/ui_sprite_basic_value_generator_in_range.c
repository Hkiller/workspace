#include <assert.h>
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite/ui_sprite_entity_attr.h"
#include "ui/sprite/ui_sprite_entity_calc.h"
#include "ui_sprite_basic_value_generator_i.h"

int ui_sprite_basic_value_generator_init_in_range(
    ui_sprite_basic_value_generator_t generator, uint16_t once_gen_count,
    ui_sprite_entity_t entity, dr_data_source_t data_srouce)
{
    double range_min;
    double range_max;

    if (ui_sprite_entity_check_calc_double(
            &range_min, generator->m_def.data.in_range.min, entity, data_srouce, generator->m_module->m_em)
        != 0)
    {
        CPE_ERROR(
            generator->m_module->m_em, "entity %d(%s): value_generator_init_in_range: calc min value from %s fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), generator->m_def.data.in_range.min);
        return -1;
    }

    if (ui_sprite_entity_check_calc_double(
            &range_max, generator->m_def.data.in_range.max, entity, data_srouce, generator->m_module->m_em)
        != 0)
    {
        CPE_ERROR(
            generator->m_module->m_em, "entity %d(%s): value_generator_init_in_range: calc max value from %s fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), generator->m_def.data.in_range.max);
        return -1;
    }

    if (once_gen_count <= 1) {
        if (ui_sprite_basic_value_generator_add_float(generator, (range_min + range_max) / 2) != 0) {
            CPE_ERROR(
                generator->m_module->m_em, "entity %d(%s): value_generator_init_in_range: add value fail!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
            return -1;
        }
    }
    else {
        uint16_t i;
        float step = (range_max - range_min) / (once_gen_count - 1);

        for(i = 0; i < once_gen_count; ++i, range_min += step) {
            if (ui_sprite_basic_value_generator_add_float(generator, range_min) != 0) {
                CPE_ERROR(
                    generator->m_module->m_em, "entity %d(%s): value_generator_init_in_range: add value fail!",
                    ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
                return -1;
            }
        }
    }

    return 0;
}
