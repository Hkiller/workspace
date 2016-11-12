#ifndef SVR_PAYMENT_DELIVER_SVR_ADAPTER_TYPE_H
#define SVR_PAYMENT_DELIVER_SVR_ADAPTER_TYPE_H
#include "payment_deliver_svr.h"

typedef int (*payment_deliver_adapter_init_fun_t)(payment_deliver_adapter_t adapter, cfg_t cfg);
typedef void (*payment_deliver_adapter_fini_fun_t)(payment_deliver_adapter_t adapter);

typedef int (*payment_deliver_adapter_on_request_fun_t)(
    payment_deliver_svr_t svr, payment_deliver_request_t request);
typedef int (*payment_deliver_adapter_on_response_fun_t)(
    payment_deliver_svr_t svr, payment_deliver_request_t request, int svr_error);

struct payment_deliver_adapter_type {
    uint8_t m_service_type;
    const char * m_service_name;
    payment_deliver_adapter_init_fun_t m_init;
    payment_deliver_adapter_fini_fun_t m_fini;
    payment_deliver_adapter_on_request_fun_t m_on_request;
    payment_deliver_adapter_on_response_fun_t m_on_response;    
};

extern struct payment_deliver_adapter_type g_adapter_types[];
extern uint8_t g_adapter_type_count;

#define PAYMENT_DECLARE_ADAPTER(__type)                                 \
    extern int payment_deliver_svr_ ## __type ## _init(                 \
        payment_deliver_adapter_t adapter, cfg_t cfg);              \
    extern void payment_deliver_svr_ ## __type ## _fini(                \
        payment_deliver_adapter_t adapter);                         \
    extern int                                                          \
    payment_deliver_svr_ ## __type ## _on_request(                      \
        payment_deliver_svr_t svr, payment_deliver_request_t request);  \
    extern int                                                          \
    payment_deliver_svr_ ## __type ## _on_response(                     \
        payment_deliver_svr_t svr, payment_deliver_request_t request,   \
        int svr_error);

#define PAYMENT_IMPL_ADAPTER(__type, __id)                      \
    { __id, #__type                                             \
            , payment_deliver_svr_ ## __type ## _init           \
            , payment_deliver_svr_ ## __type ## _fini           \
            , payment_deliver_svr_ ## __type ## _on_request     \
            , payment_deliver_svr_ ## __type ## _on_response    \
            }
    
#endif
