#include <assert.h>
#include "cpe/utils/random.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite/ui_sprite_entity_attr.h"
#include "ui/sprite/ui_sprite_entity_calc.h"
#include "ui_sprite_basic_value_generator_i.h"

int ui_sprite_basic_value_generator_init_rand(
    ui_sprite_basic_value_generator_t generator, uint16_t once_gen_count,
    ui_sprite_entity_t entity, dr_data_source_t data_srouce)
{
    double range_min;
    double range_max;
    uint16_t i;
    
    if (ui_sprite_entity_check_calc_double(
            &range_min, generator->m_def.data.rand.min, entity, data_srouce, generator->m_module->m_em)
        != 0)
    {
        CPE_ERROR(
            generator->m_module->m_em, "entity %d(%s): value_generator_init_rand: calc min value from %s fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), generator->m_def.data.rand.min);
        return -1;
    }

    if (ui_sprite_entity_check_calc_double(
            &range_max, generator->m_def.data.rand.max, entity, data_srouce, generator->m_module->m_em)
        != 0)
    {
        CPE_ERROR(
            generator->m_module->m_em, "entity %d(%s): value_generator_init_rand: calc max value from %s fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), generator->m_def.data.rand.max);
        return -1;
    }

    if (once_gen_count < 1) once_gen_count = 1;

    for(i = 0; i < once_gen_count; ++i) {
        float value = cpe_rand_ctx_generate_f_range(cpe_rand_ctx_dft(), range_min, range_max);
        if (ui_sprite_basic_value_generator_add_float(generator, value) != 0) {
            CPE_ERROR(
                generator->m_module->m_em, "entity %d(%s): value_generator_init_rand: add value fail!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
            return -1;
        }
    }

    return 0;
}
