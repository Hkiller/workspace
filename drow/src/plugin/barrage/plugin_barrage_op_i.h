#ifndef PLUGIN_BARRAGE_OP_I_H
#define PLUGIN_BARRAGE_OP_I_H
#include "plugin_barrage_env_i.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*plugin_barrage_op_fun_t) (void * ctx);

struct plugin_barrage_op {
    plugin_barrage_op_list_t * m_op_list;
    TAILQ_ENTRY(plugin_barrage_op) m_next;
    uint32_t m_op_frame;
    plugin_barrage_op_fun_t m_op_fun;
    void * m_op_ctx;
};

void plugin_barrage_op_execute(plugin_barrage_env_t env, uint8_t near_queue_pos);
void plugin_barrage_op_move_far_to_near(plugin_barrage_env_t env);

void plugin_barrage_op_enqueue(plugin_barrage_env_t env, plugin_barrage_op_t op);
void plugin_barrage_op_dequeue(plugin_barrage_env_t env, plugin_barrage_op_t op);

#ifdef __cplusplus
}
#endif

#endif
