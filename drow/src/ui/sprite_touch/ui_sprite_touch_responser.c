#include <assert.h>
#include <math.h>
#include "cpe/pal/pal_strings.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite/ui_sprite_component.h"
#include "ui_sprite_touch_responser_i.h"
#include "ui_sprite_touch_touchable_i.h"
#include "ui_sprite_touch_trace_i.h"
#include "ui_sprite_touch_mgr_i.h"


void ui_sprite_touch_responser_free(ui_sprite_touch_responser_t responser) {
    ui_sprite_fsm_action_free(ui_sprite_fsm_action_from_data(responser));
}

int ui_sprite_touch_responser_set_is_capture(ui_sprite_touch_responser_t responser, uint8_t is_capture) {
    responser->m_is_capture = is_capture;
    return 0;
}

int ui_sprite_touch_responser_set_is_grab(ui_sprite_touch_responser_t responser, uint8_t is_grab) {
    responser->m_is_grab = is_grab;
    return 0;
}

int ui_sprite_touch_responser_set_finger_count(ui_sprite_touch_responser_t responser, uint8_t finger_count) {
    if (finger_count <= 0 || finger_count > UI_SPRITE_TOUCH_MAX_FINGER_COUNT) {
        CPE_ERROR(responser->m_touchable->m_env->m_mgr->m_em, "set finger count %d error!", finger_count);
        return -1;
    }

    if (ui_sprite_fsm_action_is_active(ui_sprite_fsm_action_from_data(responser))) {
        CPE_ERROR(responser->m_touchable->m_env->m_mgr->m_em, "can`t set det finger count in active!");
        return -1;
    }

    responser->m_finger_count = finger_count;
    return 0;
}

void ui_sprite_touch_responser_cancel(ui_sprite_touch_responser_t responser) {
    ui_sprite_touch_touchable_t touchable = responser->m_touchable;
    ui_sprite_touch_mgr_t mgr = responser->m_touchable->m_env->m_mgr;
    ui_sprite_entity_t entity = ui_sprite_component_entity(ui_sprite_component_from_data(touchable));

    assert(responser->m_binding_count <= responser->m_finger_count);

    while(!TAILQ_EMPTY(&responser->m_bindings)) {
        ui_sprite_touch_responser_unbind_tracer(TAILQ_FIRST(&responser->m_bindings));
    }
    assert(responser->m_binding_count == 0);

    if (responser->m_is_start) {
        if (ui_sprite_entity_debug(entity)) {
            CPE_INFO(
                mgr->m_em, "entity %d(%s): finger %d: touch cancel",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), responser->m_finger_count);
        }

        if (responser->m_on_cancel) responser->m_on_cancel(responser);

        responser->m_is_start = 0;
    }
}

int ui_sprite_touch_responser_init(
    ui_sprite_touch_responser_t responser,
    ui_sprite_entity_t entity, ui_sprite_touch_touchable_t touchable)
{
    responser->m_touchable = touchable;
    responser->m_z = touchable->m_z;
    responser->m_finger_count = 0;
    responser->m_is_capture = 0;
    responser->m_is_grab = 0;
    responser->m_is_start = 0;
    responser->m_threshold = touchable->m_env->m_mgr->m_dft_threshold;
    responser->m_binding_count = 0;
    responser->m_on_begin = NULL;
    responser->m_on_move = NULL;
    responser->m_on_end = NULL;
    responser->m_on_cancel = NULL;

    TAILQ_INIT(&responser->m_bindings);

    TAILQ_INSERT_TAIL(&touchable->m_responsers, responser, m_next_for_touchable);

    return 0;
}

void ui_sprite_touch_responser_fini(ui_sprite_touch_responser_t responser) {
    ui_sprite_touch_touchable_t touchable = responser->m_touchable;

    assert(responser->m_binding_count == 0);

    while(!TAILQ_EMPTY(&responser->m_bindings)) {
        ui_sprite_touch_responser_unbind_tracer(TAILQ_FIRST(&responser->m_bindings));
    }

    TAILQ_REMOVE(&touchable->m_responsers, responser, m_next_for_touchable);
}

int ui_sprite_touch_responser_enter(ui_sprite_touch_responser_t responser) {
    ui_sprite_touch_env_t env = responser->m_touchable->m_env;
    ui_sprite_touch_trace_t trace;
    uint8_t inserted = 0;
    ui_sprite_touch_responser_t insert_after;

    assert(responser->m_finger_count > 0);
    assert(responser->m_binding_count == 0);

    assert(responser->m_is_start == 0);

    TAILQ_FOREACH_REVERSE(insert_after, &env->m_waiting_responsers, ui_sprite_touch_responser_list, m_next_for_env) {
        if (insert_after->m_z >= responser->m_z) {
            TAILQ_INSERT_AFTER(&env->m_waiting_responsers, insert_after, responser, m_next_for_env);
            inserted = 1;
            break;
        }
    }
    if (!inserted) {
        TAILQ_INSERT_HEAD(&env->m_waiting_responsers, responser, m_next_for_env);
    }

    TAILQ_FOREACH(trace, &env->m_touch_traces, m_next_for_env) {
        if (trace->m_is_ending) continue;
        
        ui_sprite_touch_process_trace_begin(env, trace, &trace->m_last_screen_pt, &trace->m_last_world_pt);
    }

    return 0;
}

void ui_sprite_touch_responser_exit(ui_sprite_touch_responser_t responser) {
    ui_sprite_touch_env_t env = responser->m_touchable->m_env;

    ui_sprite_touch_responser_cancel(responser);

    assert(responser->m_binding_count < responser->m_finger_count);
    assert(responser->m_is_start == 0);
    assert(responser->m_finger_count > 0);
    assert(responser->m_binding_count == 0);

    assert(!TAILQ_EMPTY(&env->m_waiting_responsers));

    TAILQ_REMOVE(&env->m_waiting_responsers, responser, m_next_for_env);
}

ui_sprite_touch_responser_binding_t
ui_sprite_touch_responser_bind_tracer(ui_sprite_touch_responser_t responser, ui_sprite_touch_trace_t trace) {
    ui_sprite_touch_touchable_t touchable = responser->m_touchable;
    ui_sprite_touch_env_t env = responser->m_touchable->m_env;
    ui_sprite_touch_mgr_t mgr = env->m_mgr;
    ui_sprite_touch_responser_binding_t binding;

    binding = mem_alloc(mgr->m_alloc, sizeof(struct ui_sprite_touch_responser_binding));
    if (binding == NULL) {
        ui_sprite_entity_t entity = ui_sprite_component_entity(ui_sprite_component_from_data(touchable));
        CPE_ERROR(
            mgr->m_em, "entity %d(%s): finger %d: alloc binding fail",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), responser->m_finger_count);
        return NULL;
    }

    bzero(binding, sizeof(*binding));
    binding->m_trace = trace;
    binding->m_responser = responser;

    TAILQ_INSERT_TAIL(&trace->m_bindings, binding, m_next_for_trace);
    TAILQ_INSERT_TAIL(&responser->m_bindings, binding, m_next_for_responser);
    responser->m_binding_count++;

    /*全部的点已经激活，则不再处理开始点击消息 */
    if (responser->m_binding_count == responser->m_finger_count) {
        ui_sprite_entity_t entity = ui_sprite_component_entity(ui_sprite_component_from_data(touchable));

        if (ui_sprite_entity_debug(entity)) {
            CPE_INFO(
                mgr->m_em, "entity %d(%s): finger %d: waiting ==> active",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), responser->m_finger_count);
        }

        TAILQ_REMOVE(&env->m_waiting_responsers, responser, m_next_for_env);
        TAILQ_INSERT_TAIL(&env->m_active_responsers, responser, m_next_for_env);
    }

    return binding;
}

void ui_sprite_touch_responser_unbind_tracer(ui_sprite_touch_responser_binding_t binding) {
    ui_sprite_touch_responser_t responser = binding->m_responser;
    ui_sprite_touch_touchable_t touchable = responser->m_touchable;
    ui_sprite_touch_env_t env = touchable->m_env;
    ui_sprite_touch_mgr_t mgr = env->m_mgr;

    /*当前移除的是最后一个点，则又开始需要关注新的touch点加入，把自己加入waiting列表 */
    assert(responser->m_binding_count > 0);
    if (responser->m_binding_count == responser->m_finger_count) {
        uint8_t inserted = 0;
        ui_sprite_touch_responser_t insert_after;
        ui_sprite_entity_t entity = ui_sprite_component_entity(ui_sprite_component_from_data(touchable));
        if (ui_sprite_entity_debug(entity)) {
            CPE_INFO(
                mgr->m_em, "entity %d(%s): finger %d: active ==> waiting",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), responser->m_finger_count);
        }

        TAILQ_REMOVE(&env->m_active_responsers, responser, m_next_for_env);

        TAILQ_FOREACH_REVERSE(insert_after, &env->m_waiting_responsers, ui_sprite_touch_responser_list, m_next_for_env) {
            if (insert_after->m_z >= responser->m_z) {
                TAILQ_INSERT_AFTER(&env->m_waiting_responsers, insert_after, responser, m_next_for_env);
                inserted = 1;
                break;
            }
        }
        if (!inserted) {
            TAILQ_INSERT_HEAD(&env->m_waiting_responsers, responser, m_next_for_env);
        }
    }

    TAILQ_REMOVE(&responser->m_bindings, binding, m_next_for_responser);
    TAILQ_REMOVE(&binding->m_trace->m_bindings, binding, m_next_for_trace);
    responser->m_binding_count--;

    mem_free(mgr->m_alloc, binding);
}

ui_sprite_touch_responser_binding_t
ui_sprite_touch_responser_binding_find(ui_sprite_touch_responser_t responser, ui_sprite_touch_trace_t trace) {
    ui_sprite_touch_responser_binding_t binding;

    TAILQ_FOREACH(binding, &responser->m_bindings, m_next_for_responser) {
        if (binding->m_trace == trace) return binding;
    }

    return NULL;
}

static void ui_sprite_touch_responser_check_start(
    ui_sprite_touch_mgr_t mgr, ui_sprite_entity_t entity,
    ui_sprite_touch_touchable_t touchable, ui_sprite_touch_responser_t responser)
{
    assert(!responser->m_is_start);

    if (responser->m_threshold <= 0) {
        if (ui_sprite_entity_debug(entity)) {
            CPE_INFO(
                mgr->m_em, "entity %d(%s): finger %d: touch begin",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), responser->m_finger_count);
        }

        if (responser->m_on_begin) responser->m_on_begin(responser);

        responser->m_is_start = 1;
    }
    else {
        ui_sprite_touch_responser_binding_t binding;
        TAILQ_FOREACH(binding, &responser->m_bindings, m_next_for_responser) {
            float distance =
                fabs(binding->m_cur_screen_pt.x - binding->m_start_screen_pt.x)
                + fabs(binding->m_cur_screen_pt.y - binding->m_start_screen_pt.y);
            if (distance < responser->m_threshold) {
                if (ui_sprite_entity_debug(entity)) {
                    CPE_INFO(
                        mgr->m_em, "entity %d(%s): finger %d: touch not active",
                        ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), responser->m_finger_count);
                }

                return;
            }
        }

        if (ui_sprite_entity_debug(entity)) {
            CPE_INFO(
                mgr->m_em, "entity %d(%s): finger %d: (threshold=%d) touch begin",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), responser->m_finger_count,
                responser->m_threshold);
        }

        if (responser->m_on_begin) responser->m_on_begin(responser);

        responser->m_is_start = 1;
    }
}

void ui_sprite_touch_responser_on_begin(ui_sprite_touch_responser_t responser) {
    ui_sprite_touch_touchable_t touchable;
    ui_sprite_touch_mgr_t mgr;
    ui_sprite_entity_t entity;

    if (responser->m_binding_count < responser->m_finger_count) return;

    if (!responser->m_is_start) {
        touchable = responser->m_touchable;
        mgr = touchable->m_env->m_mgr;
        entity = ui_sprite_component_entity(ui_sprite_component_from_data(touchable));

        ui_sprite_touch_responser_check_start(mgr, entity, touchable, responser);
    }
}

void ui_sprite_touch_responser_on_move(ui_sprite_touch_responser_t responser) {
    ui_sprite_touch_touchable_t touchable;
    ui_sprite_touch_mgr_t mgr;
    ui_sprite_entity_t entity;

    if (responser->m_binding_count < responser->m_finger_count) return;

    touchable = responser->m_touchable;
    mgr = touchable->m_env->m_mgr;
    entity = ui_sprite_component_entity(ui_sprite_component_from_data(touchable));

    if (!responser->m_is_start) {
        ui_sprite_touch_responser_check_start(mgr, entity, touchable, responser);
    }

    if (responser->m_is_start) {
        if (responser->m_on_move) responser->m_on_move(responser);
    }
}

void ui_sprite_touch_responser_on_end(ui_sprite_touch_responser_t responser) {
    ui_sprite_touch_touchable_t touchable;
    ui_sprite_touch_mgr_t mgr;
    ui_sprite_entity_t entity;

    if (!responser->m_is_start) return;
    if (responser->m_binding_count > responser->m_finger_count) return;

    touchable = responser->m_touchable;
    mgr = touchable->m_env->m_mgr;
    entity = ui_sprite_component_entity(ui_sprite_component_from_data(touchable));

    if (ui_sprite_entity_debug(entity)) {
        CPE_INFO(
            mgr->m_em, "entity %d(%s): finger %d: touch end",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), responser->m_finger_count);
    }

    if (responser->m_on_end) responser->m_on_end(responser);

    responser->m_is_start = 0;
}
