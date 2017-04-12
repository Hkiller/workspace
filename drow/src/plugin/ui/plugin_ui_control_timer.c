#include <assert.h>
#include "plugin_ui_control_timer_i.h"
#include "plugin_ui_control_i.h"
#include "plugin_ui_page_i.h"
#include "plugin_ui_env_i.h"

plugin_ui_control_timer_t
plugin_ui_control_timer_create(
    plugin_ui_control_t control, uint16_t timer_type,
    uint32_t delay, uint32_t span, uint16_t repeat,
    plugin_ui_timer_fun_t fun, void * ctx)
{
    plugin_ui_env_t env = control->m_page->m_env;
    plugin_ui_control_timer_t timer;
    
    timer = TAILQ_FIRST(&env->m_free_timers);
    if (timer) {
        TAILQ_REMOVE(&env->m_free_timers, timer, m_next);
    }
    else {
        timer = mem_alloc(env->m_module->m_alloc, sizeof(struct plugin_ui_control_timer));
        if (timer == NULL) {
            CPE_ERROR(env->m_module->m_em, "plugin_ui_control_timer_create: alloc fail!");
            return NULL;
        }
    }

    timer->m_control = control;
    timer->m_timer_type = timer_type;
    timer->m_next_left = delay;
    timer->m_span = span;
    timer->m_repeat_count = repeat;
    timer->m_fun = fun;
    timer->m_ctx = ctx;

    plugin_ui_control_timer_place(TAILQ_FIRST(&control->m_timers), timer);

    return timer;
}

void plugin_ui_control_timer_place(plugin_ui_control_timer_t insert_at, plugin_ui_control_timer_t timer) {
    while(insert_at) {
        if (insert_at->m_next_left > timer->m_next_left) {
            break;
        }
        else {
            insert_at = TAILQ_NEXT(insert_at, m_next);
        }
    }

    if (insert_at) {
        TAILQ_INSERT_BEFORE(insert_at, timer, m_next);
    }
    else {
        TAILQ_INSERT_TAIL(&timer->m_control->m_timers, timer, m_next);
    }
}

void plugin_ui_control_timer_free(plugin_ui_control_timer_t timer) {
    plugin_ui_control_t control = timer->m_control;
    plugin_ui_env_t env = control->m_page->m_env;

    TAILQ_REMOVE(&control->m_timers, timer, m_next);

    timer->m_control = (void *)env;
    TAILQ_INSERT_TAIL(&env->m_free_timers, timer, m_next);
}

void plugin_ui_control_timer_real_free(plugin_ui_control_timer_t timer) {
    plugin_ui_env_t env = (void*)timer->m_control;

    TAILQ_REMOVE(&env->m_free_timers, timer, m_next);
    mem_free(env->m_module->m_alloc, timer);
}

uint8_t plugin_ui_control_timer_data_capacity(plugin_ui_control_timer_t timer) {
    return CPE_ARRAY_SIZE(timer->m_data);
}

void * plugin_ui_control_timer_data(plugin_ui_control_timer_t timer) {
    return timer->m_data;
}

void plugin_ui_control_timer_clear(plugin_ui_control_t control) {
    while(!TAILQ_EMPTY(&control->m_timers)) {
        plugin_ui_control_timer_free(TAILQ_FIRST(&control->m_timers));
    }
}

void plugin_ui_control_timer_clear_by_type(plugin_ui_control_t control, uint16_t timer_type) {
    plugin_ui_control_timer_t timer, next;

    for(timer = TAILQ_FIRST(&control->m_timers); timer; timer = next) {
        next = TAILQ_NEXT(timer, m_next);

        if (timer->m_timer_type == timer_type) {
            plugin_ui_control_timer_free(timer);
        }
    }
    
}

void plugin_ui_control_timer_process(plugin_ui_control_t control, uint16_t duration_ms) {
    plugin_ui_control_timer_t timer;
    plugin_ui_control_timer_list_t processing_timers;

    TAILQ_INIT(&processing_timers);

    /*讲所有需要处理的定时器移动到processing*/
    while((timer = TAILQ_FIRST(&control->m_timers)) && timer->m_next_left <= duration_ms) {
        TAILQ_REMOVE(&control->m_timers, timer, m_next);
        TAILQ_INSERT_TAIL(&processing_timers, timer, m_next);
    }

    /*不需要处理的定时器，都推进间隔时间 */
    TAILQ_FOREACH(timer, &control->m_timers, m_next) {
        assert(timer->m_next_left > duration_ms);
        timer->m_next_left -= duration_ms;
    }

    /*处理所有等待处理的定时器 */
    while(!TAILQ_EMPTY(&processing_timers) && duration_ms > 0) {
        plugin_ui_control_timer_t left_timer;
        
        timer = TAILQ_FIRST(&processing_timers);

        assert(timer->m_next_left <= duration_ms);

        TAILQ_REMOVE(&processing_timers, timer, m_next);

        duration_ms -= timer->m_next_left;
        TAILQ_FOREACH(left_timer, &processing_timers, m_next) {
            assert(left_timer->m_next_left >= timer->m_next_left);
            left_timer->m_next_left -= timer->m_next_left;
        }

        timer->m_fun(timer->m_ctx ? timer->m_ctx : timer->m_data, control, timer->m_timer_type);

        /*不需要循环了，则释放 */
        if (timer->m_repeat_count == 1) {
            timer->m_control = (void*)control->m_page->m_env;
            TAILQ_INSERT_TAIL(&control->m_page->m_env->m_free_timers, timer, m_next);
            continue;
        }

        if (timer->m_repeat_count > 0) timer->m_repeat_count--;
        timer->m_next_left = timer->m_span;

        /*在本次tick中不需要再处理了，插入定时器队列 */
        if (timer->m_span > duration_ms) {
            plugin_ui_control_timer_place(TAILQ_FIRST(&control->m_timers), timer);
            continue;
        }

        /*重新插入处理队列 */
        left_timer = TAILQ_FIRST(&processing_timers);
        while(left_timer) {
            if (left_timer->m_next_left > timer->m_next_left) {
                break;
            }
            else {
                left_timer = TAILQ_NEXT(left_timer, m_next);
            }
        }

        if (left_timer) {
            TAILQ_INSERT_BEFORE(left_timer, timer, m_next);
        }
        else {
            TAILQ_INSERT_TAIL(&processing_timers, timer, m_next);
        }
    }
}
