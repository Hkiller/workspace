#ifndef PLUGIN_BARRAGE_TRIGGER_OP_I_H
#define PLUGIN_BARRAGE_TRIGGER_OP_I_H
#include "plugin_barrage_emitter_i.h"
#include "plugin_barrage_bullet_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_barrage_trigger_op {
    void * m_mgr;
    struct plugin_barrage_op m_op;
    TAILQ_ENTRY(plugin_barrage_trigger_op) m_next;
    float m_base_value;
    float m_to_value;
    uint16_t m_total_tick;
    uint16_t m_cur_tick;
    uint8_t m_result_type;
    void * m_op_target;
};

plugin_barrage_trigger_op_t
plugin_barrage_trigger_op_create(plugin_barrage_env_t env, plugin_barrage_trigger_op_list_t list);
void plugin_barrage_trigger_op_free(plugin_barrage_env_t env, plugin_barrage_trigger_op_t trigger_op);
void plugin_barrage_trigger_op_free_all(plugin_barrage_env_t env, plugin_barrage_trigger_op_list_t list);
void plugin_barrage_trigger_op_real_free(plugin_barrage_trigger_op_t trigger_op);

#ifdef __cplusplus
}
#endif

#endif
