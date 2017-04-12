#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/dr/dr_json.h"
#include "plugin_barrage_data_emitter_i.h"

/*emitter */
plugin_barrage_data_emitter_t
plugin_barrage_data_emitter_create(plugin_barrage_data_barrage_t barrage) {
    plugin_barrage_module_t module = barrage->m_module;
    plugin_barrage_data_emitter_t emitter;

    emitter = (plugin_barrage_data_emitter_t)mem_alloc(module->m_alloc, sizeof(struct plugin_barrage_data_emitter));
    if (emitter == NULL) {
        CPE_ERROR(
            module->m_em, "barrage %s: create emitter: alloc emitter fail!",
            ui_data_src_path_dump(&module->m_dump_buffer, barrage->m_src));
        return NULL;
    }

    emitter->m_barrage = barrage;
    bzero(&emitter->m_data, sizeof(emitter->m_data));

    emitter->m_emitter_trigger_count = 0;
    emitter->m_emitter_frame_triggers_begin = NULL;
    emitter->m_emitter_frame_triggers_last = NULL;
    emitter->m_emitter_check_triggers = NULL;
    emitter->m_emitter_noop_triggers = NULL;

    emitter->m_bullet_trigger_count = 0;
    emitter->m_bullet_frame_triggers_begin = NULL;
    emitter->m_bullet_frame_triggers_last = NULL;
    emitter->m_bullet_check_triggers = NULL;
    emitter->m_bullet_noop_triggers = NULL;

    TAILQ_INSERT_TAIL(&barrage->m_emitters, emitter, m_next);

    return emitter;
}

void plugin_barrage_data_emitter_free(plugin_barrage_data_emitter_t emitter) {
    plugin_barrage_data_barrage_t barrage = emitter->m_barrage;
    plugin_barrage_module_t module = barrage->m_module;

    /*emitter triggers */
    while(emitter->m_emitter_frame_triggers_begin) {
        plugin_barrage_data_emitter_trigger_free(emitter->m_emitter_frame_triggers_begin);
    }

    while(emitter->m_emitter_check_triggers) {
        plugin_barrage_data_emitter_trigger_free(emitter->m_emitter_check_triggers);
    }

    while(emitter->m_emitter_noop_triggers) {
        plugin_barrage_data_emitter_trigger_free(emitter->m_emitter_noop_triggers);
    }

    assert(emitter->m_emitter_trigger_count == 0);
    assert(emitter->m_emitter_frame_triggers_last == NULL);

    /*bullet triggers */
    while(emitter->m_bullet_frame_triggers_begin) {
        plugin_barrage_data_bullet_trigger_free(emitter->m_bullet_frame_triggers_begin);
    }

    while(emitter->m_bullet_check_triggers) {
        plugin_barrage_data_bullet_trigger_free(emitter->m_bullet_check_triggers);
    }

    while(emitter->m_bullet_noop_triggers) {
        plugin_barrage_data_bullet_trigger_free(emitter->m_bullet_noop_triggers);
    }

    assert(emitter->m_bullet_trigger_count == 0);
    assert(emitter->m_bullet_frame_triggers_last == NULL);

    TAILQ_REMOVE(&barrage->m_emitters, emitter, m_next);
    
    mem_free(module->m_alloc, emitter);
}

BARRAGE_EMITTER_INFO *
plugin_barrage_data_emitter_data(plugin_barrage_data_emitter_t emitter) {
    return &emitter->m_data;
}

float plugin_barrage_trigger_calc_value(float base_value, uint8_t op_type, float op_value) {
    switch(op_type) {
    case barrage_emitter_result_op_set:
        return op_value;
    case barrage_emitter_result_op_inc:
        return base_value + op_value;
    case barrage_emitter_result_op_dec:
        return base_value - op_value;
    default:
        return 0.0f;
    }
}

float plugin_barrage_trigger_calc_angle(float base_value, uint8_t op_type, float op_value, plugin_barrage_data_emitter_flip_type_t flip_type) {
    int8_t result_add;

    switch(op_type) {
    case barrage_emitter_result_op_set: {
        float angle = op_value;
        if (flip_type == plugin_barrage_data_emitter_flip_type_x || flip_type == plugin_barrage_data_emitter_flip_type_xy) {
            angle = 180.0f - angle;
        }

        if (flip_type == plugin_barrage_data_emitter_flip_type_y || flip_type == plugin_barrage_data_emitter_flip_type_xy) {
            angle = - angle;
        }

        return angle;
    }
    case barrage_emitter_result_op_inc: {
        result_add = 1;
        break;
    }
    case barrage_emitter_result_op_dec: {
        result_add = -1;
        break;
    }
    default:
        return 0.0f;
    }

    if (flip_type == plugin_barrage_data_emitter_flip_type_x || flip_type == plugin_barrage_data_emitter_flip_type_xy) {
        result_add = - result_add;
    }

    if (flip_type == plugin_barrage_data_emitter_flip_type_y || flip_type == plugin_barrage_data_emitter_flip_type_xy) {
        result_add = - result_add;
    }

    return result_add > 0
        ? base_value + op_value
        : base_value - op_value;
}
    
void plugin_barrage_trigger_op_value_float(float * result, uint8_t op_type, float op_value) {
    switch(op_type) {
    case barrage_emitter_result_op_set:
        *result = op_value;
        break;
    case barrage_emitter_result_op_inc:
        *result += op_value;
        break;
    case barrage_emitter_result_op_dec:
        *result -= op_value;
        break;
    }
}

void plugin_barrage_trigger_op_value_uint8(uint8_t * result, uint8_t op_type, uint8_t op_value) {
    switch(op_type) {
    case barrage_emitter_result_op_set:
        *result = op_value;
        break;
    case barrage_emitter_result_op_inc:
        *result += op_value;
        break;
    case barrage_emitter_result_op_dec:
        *result -= op_value;
        break;
    }
}

void plugin_barrage_trigger_op_value_uint16(uint16_t * result, uint8_t op_type, uint16_t op_value) {
    switch(op_type) {
    case barrage_emitter_result_op_set:
        *result = op_value;
        break;
    case barrage_emitter_result_op_inc:
        *result += op_value;
        break;
    case barrage_emitter_result_op_dec:
        *result -= op_value;
        break;
    }
}

uint8_t plugin_barrage_trigger_check_value_float(float result, uint8_t op_type, float check_value) {
    switch(op_type) {
    case barrage_emitter_condition_op_eq:
        return result == check_value;
    case barrage_emitter_condition_op_bg:
        return result > check_value;
    case barrage_emitter_condition_op_lt:
        return result < check_value;
    default:
        assert(0);
        return 0;
    }
}

uint32_t plugin_barrage_trigger_check_value_uint(uint32_t result, uint8_t op_type, uint32_t check_value) {
    switch(op_type) {
    case barrage_emitter_condition_op_eq:
        return result == check_value;
    case barrage_emitter_condition_op_bg:
        return result > check_value;
    case barrage_emitter_condition_op_lt:
        return result < check_value;
    default:
        assert(0);
        return 0;
    }
}
