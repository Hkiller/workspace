#include <assert.h>
#include "cpe/utils/string_utils.h"
#include "cpe/tl/tl_manage.h"
#include "gd/app/app_context.h"
#include "appsvr_notify_schedule_i.h"
#include "appsvr_notify_activate_i.h"
#include "appsvr_notify_tag_schedule_i.h"
#include "appsvr_notify_tag_adapter_i.h"

struct appsvr_notify_schedule_add_tag_ctx {
    appsvr_notify_schedule_t m_schedule;
    int m_rv;
};

static void appsvr_notify_schedule_add_to_tag(void * i_ctx, const char * value) {
    struct appsvr_notify_schedule_add_tag_ctx * ctx = i_ctx;
    appsvr_notify_tag_t tag;

    tag = appsvr_notify_tag_check_create(ctx->m_schedule->m_module, value);
    if (tag == NULL) {
        ctx->m_rv = -1;
        return;
    }

    if (appsvr_notify_tag_schedule_create(tag, ctx->m_schedule) == NULL) {
        ctx->m_rv = -1;
        return;
    }
}

appsvr_notify_schedule_t
appsvr_notify_schedule_create(appsvr_notify_module_t module, const char * tags) {
    appsvr_notify_schedule_t schedule;
    struct appsvr_notify_schedule_add_tag_ctx add_tag_ctx;
    
    if (tags == NULL || tags[0] == 0) tags = "default";

    schedule = TAILQ_FIRST(&module->m_free_schedules);
    if (schedule) {
        TAILQ_REMOVE(&module->m_free_schedules, schedule, m_next_for_module);
    }
    else {
        schedule = mem_calloc(module->m_alloc, sizeof(struct appsvr_notify_schedule));
        if (schedule == NULL) {
            CPE_ERROR(module->m_em, "ad: create schedule: alloc fail!");
            return NULL;
        }
    }

    schedule->m_module = module;
    schedule->m_state = appsvr_notify_schedule_init;
    schedule->m_id = module->m_schedule_max_id + 1;
    schedule->m_title = NULL;
    schedule->m_content = NULL;
    schedule->m_start_time = 0;
    schedule->m_repeat_time = 0;

    TAILQ_INIT(&schedule->m_tags);
    TAILQ_INIT(&schedule->m_activates);
    
    module->m_schedule_count++;
    TAILQ_INSERT_TAIL(&module->m_schedules, schedule, m_next_for_module);
    TAILQ_INSERT_TAIL(&module->m_schedules_to_process, schedule, m_next_for_state);

    module->m_schedule_max_id++;

    /*绑定所有tag */
    add_tag_ctx.m_schedule = schedule;
    add_tag_ctx.m_rv = 0;
    cpe_str_list_for_each(tags, ':', appsvr_notify_schedule_add_to_tag, &add_tag_ctx);
    if (add_tag_ctx.m_rv != 0) {
        appsvr_notify_schedule_free(schedule);
        return NULL;
    }
    
    return schedule;
}

void appsvr_notify_schedule_free(appsvr_notify_schedule_t schedule) {
    appsvr_notify_module_t module;

    module = schedule->m_module;

    while(!TAILQ_EMPTY(&schedule->m_activates)) {
        appsvr_notify_activate_free(TAILQ_FIRST(&schedule->m_activates));
    }
    
    while(!TAILQ_EMPTY(&schedule->m_tags)) {
        appsvr_notify_tag_schedule_free(TAILQ_FIRST(&schedule->m_tags));
    }

    if (schedule->m_title) {
        mem_free(module->m_alloc, schedule->m_title);
        schedule->m_title = NULL;
    }

    if (schedule->m_content) {
        mem_free(module->m_alloc, schedule->m_content);
        schedule->m_content;
    }

    if (schedule->m_state != appsvr_notify_schedule_activated) {
        TAILQ_REMOVE(&module->m_schedules_to_process, schedule, m_next_for_state);
    }
    
    module->m_schedule_count--;
    TAILQ_REMOVE(&module->m_schedules, schedule, m_next_for_module);

    TAILQ_INSERT_TAIL(&module->m_free_schedules, schedule, m_next_for_module);
}

void appsvr_notify_schedule_real_free(appsvr_notify_schedule_t schedule) {
    TAILQ_REMOVE(&schedule->m_module->m_free_schedules, schedule, m_next_for_module);
    mem_free(schedule->m_module->m_alloc, schedule);
}

appsvr_notify_schedule_t
appsvr_notify_schedule_find_by_id(appsvr_notify_module_t module, uint32_t id) {
    appsvr_notify_schedule_t schedule;

    TAILQ_FOREACH(schedule, &module->m_schedules, m_next_for_module) {
        if (schedule->m_id == id) return schedule;
    }
    
    return NULL;
}

uint32_t appsvr_notify_schedule_id(appsvr_notify_schedule_t schedule) {
    return schedule->m_id;
}

static void appsvr_notify_schedule_set_updated(appsvr_notify_schedule_t schedule) {
    if (schedule->m_state == appsvr_notify_schedule_activated) {
        schedule->m_state = appsvr_notify_schedule_changed;
        TAILQ_INSERT_TAIL(&schedule->m_module->m_schedules_to_process, schedule, m_next_for_state);
    }
}

int appsvr_notify_schedule_set_title(appsvr_notify_schedule_t schedule, const char * title) {
    appsvr_notify_module_t module = schedule->m_module;
    void * new_title = NULL;
    
    if (title) {
        new_title = cpe_str_mem_dup(schedule->m_module->m_alloc, title);
        if (new_title == NULL) {
            CPE_ERROR(module->m_em, "appsvr_notify_schedule_set_title: dup title fail!");
            return -1;
        }
    }
    
    if (schedule->m_title) {
        mem_free(schedule->m_module->m_alloc, schedule->m_title);
    }

    schedule->m_title = new_title;
    appsvr_notify_schedule_set_updated(schedule);
    return 0;
}

const char * appsvr_notify_schedule_title(appsvr_notify_schedule_t schedule) {
    return schedule->m_title;
}

int appsvr_notify_schedule_set_content(appsvr_notify_schedule_t schedule, const char * content) {
    appsvr_notify_module_t module = schedule->m_module;
    void * new_content = NULL;

    if (content) {
        new_content = cpe_str_mem_dup(schedule->m_module->m_alloc, content);
        if (new_content == NULL) {
            CPE_ERROR(module->m_em, "appsvr_notify_schedule_set_content: dup content fail!");
            return -1;
        }
    }
    
    if (schedule->m_content) {
        mem_free(module->m_alloc, schedule->m_content);
    }

    schedule->m_content = new_content;
    appsvr_notify_schedule_set_updated(schedule);
    return 0;
}

const char * appsvr_notify_schedule_context(appsvr_notify_schedule_t schedule) {
    return schedule->m_content;
}

int appsvr_notify_schedule_set_start_at(appsvr_notify_schedule_t schedule, uint32_t start_at) {
    schedule->m_start_time = start_at;
    appsvr_notify_schedule_set_updated(schedule);
    return 0;
}

int appsvr_notify_schedule_set_start_delay(appsvr_notify_schedule_t schedule, uint32_t delay) {
    schedule->m_start_time = tl_manage_time_sec(gd_app_tl_mgr(schedule->m_module->m_app)) + delay;
    appsvr_notify_schedule_set_updated(schedule);
    return 0;
}

uint32_t appsvr_notify_schedule_start_time(appsvr_notify_schedule_t schedule) {
    return schedule->m_start_time;
}

int appsvr_notify_schedule_set_repeat_time(appsvr_notify_schedule_t schedule, uint32_t repeat_time) {
    schedule->m_repeat_time = repeat_time;
    appsvr_notify_schedule_set_updated(schedule);
    return 0;
}

uint32_t appsvr_notify_schedule_repeat_time(appsvr_notify_schedule_t schedule) {
    return schedule->m_repeat_time;
}

uint8_t appsvr_notify_schedule_is_active_on(appsvr_notify_schedule_t schedule, appsvr_notify_adapter_t adapter) {
    appsvr_notify_activate_t activate;

    TAILQ_FOREACH(activate, &schedule->m_activates, m_next_for_schedule) {
        if (activate->m_adapter == adapter) return 1;
    }

    return 0;
}

static int appsvr_notify_activate_validate(appsvr_notify_schedule_t schedule) {
    if (schedule->m_start_time == 0) {
        CPE_ERROR(schedule->m_module->m_em, "appsvr_notify_activate_validate: %d: no start time!", schedule->m_id);
        return -1;
    }

    if (schedule->m_title == NULL) {
        CPE_ERROR(schedule->m_module->m_em, "appsvr_notify_activate_validate: %d: no title!", schedule->m_id);
        return -1;
    }
    
    return 0;
}

void appsvr_notify_schedule_tick(appsvr_notify_module_t module) {
    appsvr_notify_schedule_t schedule, next_schedule;
    appsvr_notify_tag_schedule_t tag_schedule;
    appsvr_notify_tag_adapter_t tag_adapter;
    appsvr_notify_activate_t activate;
    
    for(schedule = TAILQ_FIRST(&module->m_schedules_to_process); schedule; schedule = next_schedule) {
        next_schedule = TAILQ_NEXT(schedule, m_next_for_state);

        assert(schedule->m_state != appsvr_notify_schedule_activated);

        /*验证数据正确性 */
        if (appsvr_notify_activate_validate(schedule) != 0) {
            appsvr_notify_schedule_free(schedule);
            continue;
        }
        
        /*更新所有已经安装好的schedule */
        if (schedule->m_state == appsvr_notify_schedule_changed) {
            TAILQ_FOREACH(activate, &schedule->m_activates, m_next_for_schedule) {
                activate->m_adapter->m_update_fun(activate->m_adapter->m_ctx, schedule);
            }
        }

        /*处理新增的adapter */
        TAILQ_FOREACH(tag_schedule, &schedule->m_tags, m_next_for_schedule) {
            /*遍历关联tag上的所有adapter */
            TAILQ_FOREACH(tag_adapter, &tag_schedule->m_tag->m_adapters, m_next_for_tag) {
                if (appsvr_notify_schedule_is_active_on(schedule, tag_adapter->m_adapter)) continue; /*已经激活则忽略 */
                appsvr_notify_activate_create(schedule, tag_adapter->m_adapter);
            }
        }

        /*处理完成，设置为已经激活 */
        schedule->m_state = appsvr_notify_schedule_activated;
        TAILQ_REMOVE(&module->m_schedules_to_process, schedule, m_next_for_state);
    }
}
