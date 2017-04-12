#include "appsvr_notify_tag_adapter_i.h"

appsvr_notify_tag_adapter_t
appsvr_notify_tag_adapter_create(appsvr_notify_tag_t tag, appsvr_notify_adapter_t adapter) {
    appsvr_notify_module_t module = tag->m_module;
    appsvr_notify_tag_adapter_t tag_adapter;

    tag_adapter = mem_alloc(module->m_alloc, sizeof(struct appsvr_notify_tag_adapter));
    if (tag_adapter == NULL) {
        CPE_ERROR(module->m_em, "appsvr_notify_tag_adapter_create: alloc fail!");
        return NULL;
    }

    tag_adapter->m_tag = tag;
    tag_adapter->m_adapter = adapter;

    TAILQ_INSERT_TAIL(&tag->m_adapters, tag_adapter, m_next_for_tag);
    TAILQ_INSERT_TAIL(&adapter->m_tags, tag_adapter, m_next_for_adapter);

    return tag_adapter;
}

void appsvr_notify_tag_adapter_free(appsvr_notify_tag_adapter_t tag_adapter) {
    appsvr_notify_module_t module = tag_adapter->m_tag->m_module;

    TAILQ_REMOVE(&tag_adapter->m_tag->m_adapters, tag_adapter, m_next_for_tag);
    TAILQ_REMOVE(&tag_adapter->m_adapter->m_tags, tag_adapter, m_next_for_adapter);

    mem_free(module->m_alloc, tag_adapter);
}


