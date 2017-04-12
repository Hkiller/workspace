#ifndef APPSVR_NOTIFY_TAG_SCHEDULE_I_H
#define APPSVR_NOTIFY_TAG_SCHEDULE_I_H
#include "appsvr_notify_tag_i.h"
#include "appsvr_notify_schedule_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct appsvr_notify_tag_schedule {
    appsvr_notify_tag_t m_tag;
    TAILQ_ENTRY(appsvr_notify_tag_schedule) m_next_for_tag;
    appsvr_notify_schedule_t m_schedule;
    TAILQ_ENTRY(appsvr_notify_tag_schedule) m_next_for_schedule;
};

appsvr_notify_tag_schedule_t appsvr_notify_tag_schedule_create(appsvr_notify_tag_t tag, appsvr_notify_schedule_t schedule);
void appsvr_notify_tag_schedule_free(appsvr_notify_tag_schedule_t tag_schedule);
    
void appsvr_notify_tag_schedule_real_free(appsvr_notify_tag_schedule_t tag_schedule);

#ifdef __cplusplus
}
#endif

#endif
