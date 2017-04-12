#include <assert.h>
#include "plugin_barrage_op_i.h"

void plugin_barrage_op_enqueue(plugin_barrage_env_t env, plugin_barrage_op_t op) {
    uint32_t delay_frame;

    assert(op->m_op_list == NULL);
    assert(op->m_op_frame >= env->m_cur_frame);
    assert(op->m_op_fun);

    delay_frame = op->m_op_frame - env->m_cur_frame;

    if (delay_frame < PLUGIN_BARRAGE_OP_QUEUE_COUNT) {
        uint8_t near_op_pos = (env->m_near_op_pos + delay_frame) % PLUGIN_BARRAGE_OP_QUEUE_COUNT;
        op->m_op_list = &env->m_near_ops[near_op_pos];
        TAILQ_INSERT_TAIL(op->m_op_list, op, m_next);
    }
    else {
        op->m_op_list = &env->m_far_ops;
        TAILQ_INSERT_TAIL(op->m_op_list, op, m_next);
    }

    env->m_op_count++;
}

void plugin_barrage_op_dequeue(plugin_barrage_env_t env, plugin_barrage_op_t op) {
    assert(op->m_op_frame >= env->m_cur_frame);
    assert(op->m_op_fun);
    assert(op->m_op_list);

    env->m_op_count--;
    TAILQ_REMOVE(op->m_op_list, op, m_next);

    op->m_op_list = NULL;
    op->m_op_frame = 0;
    op->m_op_fun = NULL;
    op->m_op_ctx = NULL;
}

void plugin_barrage_op_execute(plugin_barrage_env_t env, uint8_t near_queue_pos) {
    plugin_barrage_op_list_t * op_list;

    assert(near_queue_pos < CPE_ARRAY_SIZE(env->m_near_ops));

    op_list = &env->m_near_ops[near_queue_pos];

    while(!TAILQ_EMPTY(op_list)) {
        plugin_barrage_op_t cur_op = TAILQ_FIRST(op_list);
        plugin_barrage_op_fun_t cur_op_fun;
        void * cur_op_ctx;

        env->m_op_count--;
        TAILQ_REMOVE(op_list, cur_op, m_next);

        assert(cur_op->m_op_list == op_list);
        assert(cur_op->m_op_frame == env->m_cur_frame);
        assert(cur_op->m_op_fun);

        cur_op_fun = cur_op->m_op_fun;
        cur_op_ctx = cur_op->m_op_ctx;

        cur_op->m_op_list = NULL;
        cur_op->m_op_frame = 0;
        cur_op->m_op_fun = NULL;
        cur_op->m_op_ctx = NULL;

        cur_op_fun(cur_op_ctx);
    }
}

void plugin_barrage_op_move_far_to_near(plugin_barrage_env_t env) {
    plugin_barrage_op_t cur_op, next_op;

    for(cur_op = TAILQ_FIRST(&env->m_far_ops); cur_op; cur_op = next_op) {
        uint32_t delay_frame;
        uint8_t near_op_pos;

        next_op = TAILQ_NEXT(cur_op, m_next);

        assert(cur_op->m_op_list == &env->m_far_ops);
        assert(cur_op->m_op_frame > env->m_cur_frame);
        assert(cur_op->m_op_fun);

        delay_frame = cur_op->m_op_frame - env->m_cur_frame;
        if (delay_frame > PLUGIN_BARRAGE_OP_QUEUE_COUNT) continue;

        near_op_pos = (env->m_near_op_pos + delay_frame) % PLUGIN_BARRAGE_OP_QUEUE_COUNT;
        cur_op->m_op_list = &env->m_near_ops[near_op_pos];

        assert(TAILQ_EMPTY(cur_op->m_op_list) || TAILQ_FIRST(cur_op->m_op_list)->m_op_frame == cur_op->m_op_frame);
        
        TAILQ_REMOVE(&env->m_far_ops, cur_op, m_next);
        TAILQ_INSERT_TAIL(cur_op->m_op_list, cur_op, m_next);
    }
}
