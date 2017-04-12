#ifndef APPSVR_NOTIFY_SCHEDULE_H
#define APPSVR_NOTIFY_SCHEDULE_H
#include "appsvr_notify_types.h"

#ifdef __cplusplus
extern "C" {
#endif

appsvr_notify_schedule_t
appsvr_notify_schedule_create(appsvr_notify_module_t module, const char * tags);

void appsvr_notify_schedule_free(appsvr_notify_schedule_t schedule);

uint32_t appsvr_notify_schedule_id(appsvr_notify_schedule_t schedule);

int appsvr_notify_schedule_set_title(appsvr_notify_schedule_t schedule, const char * title);
const char * appsvr_notify_schedule_title(appsvr_notify_schedule_t schedule);
    
int appsvr_notify_schedule_set_content(appsvr_notify_schedule_t schedule, const char * content);
const char * appsvr_notify_schedule_context(appsvr_notify_schedule_t schedule);

int appsvr_notify_schedule_set_start_at(appsvr_notify_schedule_t schedule, uint32_t start_at);
int appsvr_notify_schedule_set_start_delay(appsvr_notify_schedule_t schedule, uint32_t delay);
uint32_t appsvr_notify_schedule_start_time(appsvr_notify_schedule_t schedule);

int appsvr_notify_schedule_set_repeat_time(appsvr_notify_schedule_t schedule, uint32_t repeat_time);
uint32_t appsvr_notify_schedule_repeat_time(appsvr_notify_schedule_t schedule);
    
#ifdef __cplusplus
}
#endif

#endif
