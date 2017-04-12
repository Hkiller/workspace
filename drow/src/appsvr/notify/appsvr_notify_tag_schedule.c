#include "appsvr_notify_tag_schedule_i.h"

appsvr_notify_tag_schedule_t
appsvr_notify_tag_schedule_create(appsvr_notify_tag_t tag, appsvr_notify_schedule_t schedule) {
    appsvr_notify_module_t module = tag->m_module;
    appsvr_notify_tag_schedule_t tag_schedule;

    tag_schedule = TAILQ_FIRST(&module->m_free_tag_schedules);
    if (tag_schedule) {
        TAILQ_REMOVE(&module->m_free_tag_schedules, tag_schedule, m_next_for_tag);
    }
    else {
        tag_schedule = mem_alloc(module->m_alloc, sizeof(struct appsvr_notify_tag_schedule));
        if (tag_schedule == NULL) {
            CPE_ERROR(module->m_em, "appsvr_notify_tag_schedule_create: alloc fail!");
            return NULL;
        }
    }

    tag_schedule->m_tag = tag;
    tag_schedule->m_schedule = schedule;

    TAILQ_INSERT_TAIL(&tag->m_schedules, tag_schedule, m_next_for_tag);
    TAILQ_INSERT_TAIL(&schedule->m_tags, tag_schedule, m_next_for_schedule);

    return tag_schedule;
}

void appsvr_notify_tag_schedule_free(appsvr_notify_tag_schedule_t tag_schedule) {
    appsvr_notify_module_t module = tag_schedule->m_tag->m_module;

    TAILQ_REMOVE(&tag_schedule->m_tag->m_schedules, tag_schedule, m_next_for_tag);
    TAILQ_REMOVE(&tag_schedule->m_schedule->m_tags, tag_schedule, m_next_for_schedule);

    tag_schedule->m_tag = (void*)module;
    TAILQ_INSERT_TAIL(&module->m_free_tag_schedules, tag_schedule, m_next_for_tag);
}

void appsvr_notify_tag_schedule_real_free(appsvr_notify_tag_schedule_t tag_schedule) {
    appsvr_notify_module_t module = (appsvr_notify_module_t)tag_schedule->m_tag;

    TAILQ_REMOVE(&module->m_free_tag_schedules, tag_schedule, m_next_for_tag);
    mem_free(module->m_alloc, tag_schedule);
}

