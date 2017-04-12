#ifndef PLUGIN_BARRAGE_DATA_EMITTER_I_H
#define PLUGIN_BARRAGE_DATA_EMITTER_I_H
#include "plugin/barrage/plugin_barrage_data_emitter.h"
#include "plugin_barrage_data_barrage_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_barrage_data_emitter {
    plugin_barrage_data_barrage_t m_barrage;
    TAILQ_ENTRY(plugin_barrage_data_emitter) m_next;
    BARRAGE_EMITTER_INFO m_data;

    uint16_t m_emitter_trigger_count;
    plugin_barrage_data_emitter_trigger_t m_emitter_frame_triggers_begin;
    plugin_barrage_data_emitter_trigger_t m_emitter_frame_triggers_last;
    plugin_barrage_data_emitter_trigger_t m_emitter_check_triggers;
    plugin_barrage_data_emitter_trigger_t m_emitter_noop_triggers;

    uint16_t m_bullet_trigger_count;
    plugin_barrage_data_bullet_trigger_t m_bullet_frame_triggers_begin;
    plugin_barrage_data_bullet_trigger_t m_bullet_frame_triggers_last;
    plugin_barrage_data_bullet_trigger_t m_bullet_check_triggers;
    plugin_barrage_data_bullet_trigger_t m_bullet_noop_triggers;
};

enum plugin_barrage_data_emitter_trigger_queue_state {
    plugin_barrage_data_emitter_trigger_queue_none = 0
    , plugin_barrage_data_emitter_trigger_queue_frame = 1
    , plugin_barrage_data_emitter_trigger_queue_check = 2
    , plugin_barrage_data_emitter_trigger_queue_noop = 3
};

struct plugin_barrage_data_emitter_trigger {
    plugin_barrage_data_emitter_t m_emitter;
    enum plugin_barrage_data_emitter_trigger_queue_state m_state;
    BARRAGE_EMITTER_EMITTER_TRIGGER_INFO m_data;
    plugin_barrage_data_emitter_trigger_t m_next;
};

struct plugin_barrage_data_bullet_trigger {
    plugin_barrage_data_emitter_t m_emitter;
    enum plugin_barrage_data_emitter_trigger_queue_state m_state;
    BARRAGE_EMITTER_BULLET_TRIGGER_INFO m_data;
    plugin_barrage_data_bullet_trigger_t m_next;
};
    
/*trigger support ops*/
void plugin_barrage_trigger_op_value_float(float * result, uint8_t op_type, float op_value);
void plugin_barrage_trigger_op_value_uint8(uint8_t * result, uint8_t op_type, uint8_t op_value);
void plugin_barrage_trigger_op_value_uint16(uint16_t * result, uint8_t op_type, uint16_t op_value);
uint8_t plugin_barrage_trigger_check_value_float(float result, uint8_t op_type, float check_value);
uint32_t plugin_barrage_trigger_check_value_uint(uint32_t result, uint8_t op_type, uint32_t check_value);

float plugin_barrage_trigger_calc_value(float base_value, uint8_t op_type, float op_value);
float plugin_barrage_trigger_calc_angle(float base_value, uint8_t op_type, float op_value, plugin_barrage_data_emitter_flip_type_t flip_type);

#ifdef __cplusplus
}
#endif

#endif
