#ifndef APPSVR_NOTIFY_TAG_ADAPTER_I_H
#define APPSVR_NOTIFY_TAG_ADAPTER_I_H
#include "appsvr_notify_tag_i.h"
#include "appsvr_notify_adapter_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct appsvr_notify_tag_adapter {
    appsvr_notify_tag_t m_tag;
    TAILQ_ENTRY(appsvr_notify_tag_adapter) m_next_for_tag;
    appsvr_notify_adapter_t m_adapter;
    TAILQ_ENTRY(appsvr_notify_tag_adapter) m_next_for_adapter;
};

appsvr_notify_tag_adapter_t appsvr_notify_tag_adapter_create(appsvr_notify_tag_t tag, appsvr_notify_adapter_t adapter);
void appsvr_notify_tag_adapter_free(appsvr_notify_tag_adapter_t tag_adapter);
    
#ifdef __cplusplus
}
#endif

#endif
