#ifndef SVR_PAYMENT_SVR_ADAPTER_TYPE_H
#define SVR_PAYMENT_SVR_ADAPTER_TYPE_H
#include "payment_svr.h"

typedef int (*payment_svr_adapter_init_fun_t)(payment_svr_adapter_t adapter, cfg_t cfg);
typedef void (*payment_svr_adapter_fini_fun_t)(payment_svr_adapter_t adapter);

typedef int (*payment_svr_adapter_send_fun_t)(
    logic_context_t ctx, logic_stack_node_t stack, payment_svr_adapter_t adapter,
    PAYMENT_RECHARGE_RECORD * record, SVR_PAYMENT_REQ_RECHARGE_COMMIT const * req);

typedef int (*payment_svr_adapter_recv_fun_t)(
    logic_context_t ctx, logic_stack_node_t stack, logic_require_t require, payment_svr_adapter_t adapter,
    PAYMENT_RECHARGE_RECORD * record, SVR_PAYMENT_REQ_RECHARGE_COMMIT const * req);

struct payment_svr_adapter_type {
    uint8_t m_service_type;
    const char * m_service_name;
    payment_svr_adapter_init_fun_t m_init;
    payment_svr_adapter_fini_fun_t m_fini;
    payment_svr_adapter_send_fun_t m_send;
    payment_svr_adapter_recv_fun_t m_recv;    
};

extern struct payment_svr_adapter_type g_adapter_types[];
extern uint8_t g_adapter_type_count;

#define PAYMENT_DECLARE_ADAPTER(__type)                                 \
    extern int payment_svr_ ## __type ## _init(payment_svr_adapter_t adapter, cfg_t cfg); \
    extern void payment_svr_ ## __type ## _fini(payment_svr_adapter_t adapter); \
    extern int                                                          \
    payment_svr_ ## __type ## _charge_send(                             \
        logic_context_t ctx, logic_stack_node_t stack,                  \
        payment_svr_adapter_t adapter,                                  \
        PAYMENT_RECHARGE_RECORD * record,                               \
        SVR_PAYMENT_REQ_RECHARGE_COMMIT const * req);                   \
    extern int                                                          \
    payment_svr_ ## __type ## _charge_recv(                             \
        logic_context_t ctx, logic_stack_node_t stack, logic_require_t require, \
        payment_svr_adapter_t adapter,                                  \
        PAYMENT_RECHARGE_RECORD * record,                               \
        SVR_PAYMENT_REQ_RECHARGE_COMMIT const * req)                    \

#define PAYMENT_IMPL_ADAPTER(__type, __id)      \
        { __id, #__type \
                , payment_svr_ ## __type ## _init \
                , payment_svr_ ## __type ## _fini \
                , payment_svr_ ## __type ## _charge_send \
                , payment_svr_ ## __type ## _charge_recv \
                }
    
#endif
