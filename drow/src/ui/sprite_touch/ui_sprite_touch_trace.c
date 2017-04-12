#include "cpe/tl/tl_manage.h"
#include "gd/app/app_context.h"
#include "ui_sprite_touch_trace_i.h"
#include "ui_sprite_touch_responser_i.h"

ui_sprite_touch_trace_t
ui_sprite_touch_trace_create(ui_sprite_touch_env_t env, int32_t id, ui_vector_2_t screen_pt, ui_vector_2_t world_pt) {
    ui_sprite_touch_trace_t trace;

    trace = mem_alloc(env->m_mgr->m_alloc, sizeof(struct ui_sprite_touch_trace));
    if (trace == NULL) {
        CPE_ERROR(env->m_mgr->m_em, "alloc touch trace fail!");
        return NULL;
    }

    trace->m_id = id;
    trace->m_is_ending = 0;
    trace->m_last_screen_pt = *screen_pt;
    trace->m_last_world_pt = *world_pt;
    TAILQ_INIT(&trace->m_bindings);

    TAILQ_INSERT_TAIL(&env->m_touch_traces, trace, m_next_for_env);

    return trace;
}

void ui_sprite_touch_trace_free(ui_sprite_touch_env_t env, ui_sprite_touch_trace_t trace) {
    TAILQ_REMOVE(&env->m_touch_traces, trace, m_next_for_env);

    while(!TAILQ_EMPTY(&trace->m_bindings)) {
        ui_sprite_touch_responser_unbind_tracer(TAILQ_FIRST(&trace->m_bindings));
    }

    mem_free(env->m_mgr->m_alloc, trace);
}

void ui_sprite_touch_trace_free_all(ui_sprite_touch_env_t env) {
    while(!TAILQ_EMPTY(&env->m_touch_traces)) {
        ui_sprite_touch_trace_free(env, TAILQ_FIRST(&env->m_touch_traces));
    }
}

ui_sprite_touch_trace_t ui_sprite_touch_trace_find(ui_sprite_touch_env_t env, int32_t id) {
    ui_sprite_touch_trace_t trace;

    TAILQ_FOREACH(trace, &env->m_touch_traces, m_next_for_env) {
        if (trace->m_id == id) return trace;
    }

    return NULL;
}
