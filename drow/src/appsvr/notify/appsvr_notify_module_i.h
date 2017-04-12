#ifndef APPSVR_NOTIFY_MODULE_I_H
#define APPSVR_NOTIFY_MODULE_I_H
#include "cpe/utils/memory.h"
#include "cpe/utils/error.h"
#include "cpe/utils/hash.h"
#include "cpe/utils/hash_string.h"
#include "cpe/utils/buffer.h"
#include "gd/app/app_types.h"
#include "appsvr/notify/appsvr_notify_module.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct appsvr_notify_tag_schedule * appsvr_notify_tag_schedule_t;
typedef struct appsvr_notify_tag_adapter * appsvr_notify_tag_adapter_t;
typedef struct appsvr_notify_activate * appsvr_notify_activate_t;
    
typedef TAILQ_HEAD(appsvr_notify_adapter_list, appsvr_notify_adapter) appsvr_notify_adapter_list_t;
typedef TAILQ_HEAD(appsvr_notify_schedule_list, appsvr_notify_schedule) appsvr_notify_schedule_list_t;
typedef TAILQ_HEAD(appsvr_notify_activate_list, appsvr_notify_activate) appsvr_notify_activate_list_t;

typedef TAILQ_HEAD(appsvr_notify_tag_list, appsvr_notify_tag) appsvr_notify_tag_list_t;
typedef TAILQ_HEAD(appsvr_notify_tag_schedule_list, appsvr_notify_tag_schedule) appsvr_notify_tag_schedule_list_t;
typedef TAILQ_HEAD(appsvr_notify_tag_adapter_list, appsvr_notify_tag_adapter) appsvr_notify_tag_adapter_list_t;
    
struct appsvr_notify_module {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    uint8_t m_debug;

    appsvr_notify_adapter_list_t m_adapters;

    appsvr_notify_tag_list_t m_tags;
    appsvr_notify_tag_schedule_list_t m_free_tag_schedules;
    
    uint32_t m_schedule_max_id;
    uint32_t m_schedule_count;
    appsvr_notify_schedule_list_t m_schedules_to_process;
    appsvr_notify_schedule_list_t m_schedules;
    appsvr_notify_schedule_list_t m_free_schedules;

    appsvr_notify_activate_list_t m_free_activates;
};

#ifdef __cplusplus
}
#endif

#endif
