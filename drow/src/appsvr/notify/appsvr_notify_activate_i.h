#ifndef APPSVR_NOTIFY_ACTIVATE_I_H
#define APPSVR_NOTIFY_ACTIVATE_I_H
#include "appsvr_notify_schedule_i.h"
#include "appsvr_notify_adapter_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct appsvr_notify_activate {
    appsvr_notify_schedule_t m_schedule;
    TAILQ_ENTRY(appsvr_notify_activate) m_next_for_schedule;
    appsvr_notify_adapter_t m_adapter;
    TAILQ_ENTRY(appsvr_notify_activate) m_next_for_adapter;
};

appsvr_notify_activate_t appsvr_notify_activate_create(appsvr_notify_schedule_t schedule, appsvr_notify_adapter_t adapter);
void appsvr_notify_activate_free(appsvr_notify_activate_t activate);
    
void appsvr_notify_activate_real_free(appsvr_notify_activate_t activate);

#ifdef __cplusplus
}
#endif

#endif
