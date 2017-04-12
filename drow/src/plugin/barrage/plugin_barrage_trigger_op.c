#include "cpe/pal/pal_strings.h"
#include "plugin_barrage_trigger_op_i.h"
#include "plugin_barrage_env_i.h"

plugin_barrage_trigger_op_t plugin_barrage_trigger_op_create(plugin_barrage_env_t env, plugin_barrage_trigger_op_list_t list) {
    plugin_barrage_trigger_op_t trigger_op;

    trigger_op = TAILQ_FIRST(&env->m_free_trigger_ops);
    if (trigger_op == NULL) {
        trigger_op = (plugin_barrage_trigger_op_t)mem_alloc(env->m_module->m_alloc, sizeof(struct plugin_barrage_trigger_op));
        if (trigger_op == NULL) return NULL;
    }
    else {
        TAILQ_REMOVE(&env->m_free_trigger_ops, trigger_op, m_next);
    }

    bzero(trigger_op, sizeof(*trigger_op));
    trigger_op->m_mgr = list;
    TAILQ_INSERT_TAIL(list, trigger_op, m_next);

    return trigger_op;
}

void plugin_barrage_trigger_op_free(plugin_barrage_env_t env, plugin_barrage_trigger_op_t trigger_op) {
    plugin_barrage_trigger_op_list_t list;

    if (trigger_op->m_op.m_op_fun) {
        plugin_barrage_op_dequeue(env, &trigger_op->m_op);
    }
    
    list = (plugin_barrage_trigger_op_list_t)trigger_op->m_mgr;

    TAILQ_REMOVE(list, trigger_op, m_next);

    trigger_op->m_mgr = env;
    TAILQ_INSERT_TAIL(&env->m_free_trigger_ops, trigger_op, m_next);
}

void plugin_barrage_trigger_op_free_all(plugin_barrage_env_t env, plugin_barrage_trigger_op_list_t list) {
    while(!TAILQ_EMPTY(list)) {
        plugin_barrage_trigger_op_free(env, TAILQ_FIRST(list));
    }
}

void plugin_barrage_trigger_op_real_free(plugin_barrage_trigger_op_t trigger_op) {
    plugin_barrage_env_t env = (plugin_barrage_env_t)trigger_op->m_mgr;

    TAILQ_REMOVE(&env->m_free_trigger_ops, trigger_op, m_next);

    mem_free(env->m_module->m_alloc, trigger_op);
}
