#include <assert.h>
#include "cpe/utils/string_utils.h"
#include "usf/logic/logic_executor_type.h"
#include "usf/logic_use/logic_op_register.h"
#include "usf/logic_use/logic_op_async.h"

struct logic_op_register {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    char * m_group_name;
    uint16_t m_executor_count;
    uint16_t m_executor_capacity;
    logic_executor_type_t * m_executor_types;
};

logic_op_register_t logic_op_register_create(gd_app_context_t app, const char * group_name, mem_allocrator_t alloc, error_monitor_t em) {
    logic_op_register_t op_register;

    op_register = mem_alloc(alloc, sizeof(struct logic_op_register));
    if (op_register == NULL) return NULL;

    op_register->m_app = app;
    op_register->m_alloc = alloc;
    op_register->m_em = em;

    op_register->m_group_name = NULL;
    if (group_name) {
        cpe_str_mem_dup(alloc, group_name);
        if (op_register->m_group_name == NULL) {
            mem_free(alloc, op_register);
            return NULL;
        }
    }

    op_register->m_executor_count = 0;
    op_register->m_executor_capacity = 0;
    op_register->m_executor_types = NULL;

    return op_register;
}

void logic_op_register_free(logic_op_register_t op_register) {
    if (op_register->m_executor_types) {
        uint32_t i;
        for(i = 0; i < op_register->m_executor_count; ++i) {
            logic_executor_type_t executor_type = op_register->m_executor_types[i];
            assert(executor_type);
            logic_executor_type_free(executor_type);
        }

        mem_free(op_register->m_alloc, op_register->m_executor_types);
        op_register->m_executor_types = NULL;
        op_register->m_executor_capacity = 0;
        op_register->m_executor_count = 0;
    }

    if (op_register->m_group_name) {
        mem_free(op_register->m_alloc, op_register->m_group_name);
        op_register->m_group_name = NULL;
    }

    mem_free(op_register->m_alloc, op_register);
}

int logic_op_register_create_op(
    logic_op_register_t op_register,
    const char * type_name,
    logic_op_fun_t send_fun,
    logic_op_recv_fun_t recv_fun,
    void * user_ctx,
    logic_op_ctx_fini_fun_t fini_fun)
{
    logic_executor_type_t executor_type;

    if (op_register->m_executor_count + 1 > op_register->m_executor_capacity) {
        uint16_t new_capacity = 
            op_register->m_executor_capacity < 16
            ? 16
            : op_register->m_executor_capacity * 2;
        logic_executor_type_t * new_executor_types =
            mem_alloc(op_register->m_alloc, sizeof(logic_executor_type_t) * new_capacity);
        if (new_executor_types == NULL) {
            CPE_ERROR(op_register->m_em, "logic_op_register: create op %s: alloc executor buf fail", type_name);
            return -1;
        }

        if (op_register->m_executor_types) {
            assert(op_register->m_executor_count > 0);
            memcpy(
                new_executor_types,
                op_register->m_executor_types,
                sizeof(logic_executor_type_t) * op_register->m_executor_count);
            mem_free(op_register->m_alloc, op_register->m_executor_types);
        }

        op_register->m_executor_types = new_executor_types;
        op_register->m_executor_capacity = new_capacity;
    }

    assert(send_fun);
    if (recv_fun == NULL) {
        executor_type = 
            logic_executor_type_create_global(
                op_register->m_app,
                op_register->m_group_name,
                type_name, send_fun,
                user_ctx, fini_fun,
                op_register->m_em);
        if (executor_type == NULL) {
            CPE_ERROR(op_register->m_em, "logic_op_register: create op %s: create sync op fail", type_name);
            return -1;
        }
    }
    else {
        executor_type = 
            logic_op_async_type_create(
                op_register->m_app,
                op_register->m_group_name,
                type_name, send_fun, recv_fun,
                user_ctx, fini_fun,
                op_register->m_em);
        if (executor_type == NULL) {
            CPE_ERROR(op_register->m_em, "logic_op_register: create op %s: create async op fail", type_name);
            return -1;
        }
    }

    op_register->m_executor_types[op_register->m_executor_count++] = executor_type;

    return 0;
}

int logic_op_register_create_ops(
    logic_op_register_t op_register,
    uint32_t op_def_count,
    logic_op_register_def_t op_defs,
    void * user_ctx)
{
    uint32_t i;
    for(i = 0; i < op_def_count; ++i) {
        int r = logic_op_register_create_op(
            op_register,
            op_defs[i].name, op_defs[i].send_fun, op_defs[i].recv_fun,
            user_ctx, NULL);
        if (r != 0) return r;
    }

    return 0;
}
