#ifndef APPSVR_NOTIFY_SCHEDULE_I_H
#define APPSVR_NOTIFY_SCHEDULE_I_H
#include "appsvr/notify/appsvr_notify_schedule.h"
#include "appsvr_notify_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum appsvr_notify_schedule_state {
    appsvr_notify_schedule_init, /*刚刚创建 */
    appsvr_notify_schedule_activated, /*已经激活 */
    appsvr_notify_schedule_changed, /*内容已经修改 */
} appsvr_notify_schedule_state_t;

struct appsvr_notify_schedule {
    appsvr_notify_module_t m_module;
    TAILQ_ENTRY(appsvr_notify_schedule) m_next_for_module;
    appsvr_notify_schedule_state_t m_state;
    TAILQ_ENTRY(appsvr_notify_schedule) m_next_for_state;
    appsvr_notify_tag_schedule_list_t m_tags;
    appsvr_notify_activate_list_t m_activates;
    uint32_t m_id;
    char * m_title;
    char * m_content;
    uint32_t m_start_time;
    uint32_t m_repeat_time;
};

void appsvr_notify_schedule_real_free(appsvr_notify_schedule_t schedule);    
void appsvr_notify_schedule_tick(appsvr_notify_module_t module);
uint8_t appsvr_notify_schedule_is_active_on(appsvr_notify_schedule_t schedule, appsvr_notify_adapter_t adapter);
    
#ifdef __cplusplus
}
#endif

#endif
