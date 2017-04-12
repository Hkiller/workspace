#ifndef APPSVR_NOTIFY_TAG_I_H
#define APPSVR_NOTIFY_TAG_I_H
#include "appsvr/notify/appsvr_notify_tag.h"
#include "appsvr_notify_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct appsvr_notify_tag {
    appsvr_notify_module_t m_module;
    TAILQ_ENTRY(appsvr_notify_tag) m_next_for_module;
    char m_name[64];
    appsvr_notify_tag_schedule_list_t m_schedules;
    appsvr_notify_tag_adapter_list_t m_adapters;
};

void appsvr_notify_schedule_real_free(appsvr_notify_schedule_t schedule);    

#ifdef __cplusplus
}
#endif

#endif
