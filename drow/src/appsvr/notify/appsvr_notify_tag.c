#include "cpe/utils/string_utils.h"
#include "appsvr_notify_tag_i.h"
#include "appsvr_notify_tag_schedule_i.h"
#include "appsvr_notify_tag_adapter_i.h"

appsvr_notify_tag_t
appsvr_notify_tag_create(appsvr_notify_module_t module, const char * name) {
    appsvr_notify_tag_t tag;

    tag = mem_alloc(module->m_alloc, sizeof(struct appsvr_notify_tag));
    if (tag == NULL) {
        CPE_ERROR(module->m_em, "appsvr_notify_tag_create: alloc fail!");
        return NULL;
    }

    tag->m_module = module;
    cpe_str_dup(tag->m_name, sizeof(tag->m_name), name);
    TAILQ_INIT(&tag->m_schedules);
    TAILQ_INIT(&tag->m_adapters);
    
    TAILQ_INSERT_TAIL(&module->m_tags, tag, m_next_for_module);

    return tag;
}

appsvr_notify_tag_t
appsvr_notify_tag_check_create(appsvr_notify_module_t module, const char * name) {
    appsvr_notify_tag_t tag;

    tag = appsvr_notify_tag_find(module, name);
    if (tag == NULL) {
        tag = appsvr_notify_tag_create(module, name);
    }

    return tag;
}

void appsvr_notify_tag_free(appsvr_notify_tag_t tag) {
    appsvr_notify_module_t module = tag->m_module;

    while(!TAILQ_EMPTY(&tag->m_schedules)) {
        appsvr_notify_tag_schedule_free(TAILQ_FIRST(&tag->m_schedules));
    }

    while(!TAILQ_EMPTY(&tag->m_adapters)) {
        appsvr_notify_tag_adapter_free(TAILQ_FIRST(&tag->m_adapters));
    }

    TAILQ_REMOVE(&module->m_tags, tag, m_next_for_module);

    mem_free(module->m_alloc, tag);
}

appsvr_notify_tag_t
appsvr_notify_tag_find(appsvr_notify_module_t module, const char * name) {
    appsvr_notify_tag_t tag;

    TAILQ_FOREACH(tag, &module->m_tags, m_next_for_module) {
        if (strcmp(tag->m_name, name) == 0) return tag;
    }

    return NULL;
}

void appsvr_notify_tag_clear(appsvr_notify_tag_t tag) {
    while(!TAILQ_EMPTY(&tag->m_schedules)) {
        appsvr_notify_schedule_free(TAILQ_FIRST(&tag->m_schedules)->m_schedule);
    }
}
