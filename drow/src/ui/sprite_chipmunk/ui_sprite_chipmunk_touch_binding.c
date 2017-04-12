#include <assert.h>
#include <math.h>
#include "cpe/pal/pal_strings.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite/ui_sprite_component.h"
#include "ui_sprite_chipmunk_touch_binding_i.h"
#include "ui_sprite_chipmunk_obj_i.h"
#include "ui_sprite_chipmunk_touch_trace_i.h"
#include "ui_sprite_chipmunk_module_i.h"

ui_sprite_chipmunk_touch_binding_t
ui_sprite_chipmunk_touch_binding_create(ui_sprite_chipmunk_touch_responser_t responser, ui_sprite_chipmunk_touch_trace_t trace, ui_sprite_chipmunk_obj_body_t body) {
    ui_sprite_chipmunk_obj_t obj = responser->m_obj;
    ui_sprite_chipmunk_env_t env = responser->m_obj->m_env;
    ui_sprite_chipmunk_module_t module = env->m_module;
    ui_sprite_chipmunk_touch_binding_t binding;

    binding = (ui_sprite_chipmunk_touch_binding_t)mem_alloc(module->m_alloc, sizeof(struct ui_sprite_chipmunk_touch_binding));
    if (binding == NULL) {
        ui_sprite_entity_t entity = ui_sprite_component_entity(ui_sprite_component_from_data(obj));
        CPE_ERROR(
            module->m_em, "entity %d(%s): finger %d: alloc binding fail",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), responser->m_finger_count);
        return NULL;
    }

    bzero(binding, sizeof(*binding));
    binding->m_trace = trace;
    binding->m_body = body;
    binding->m_responser = responser;

    TAILQ_INSERT_TAIL(&trace->m_bindings, binding, m_next_for_trace);
    TAILQ_INSERT_TAIL(&responser->m_bindings, binding, m_next_for_responser);
    responser->m_binding_count++;

    /*全部的点已经激活，则不再处理开始点击消息 */
    if (responser->m_binding_count == responser->m_finger_count) {
        ui_sprite_entity_t entity = ui_sprite_component_entity(ui_sprite_component_from_data(obj));

        if (ui_sprite_entity_debug(entity)) {
            CPE_INFO(
                module->m_em, "entity %d(%s): finger %d: waiting ==> active",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), responser->m_finger_count);
        }
    }

    return binding;
}

void ui_sprite_chipmunk_touch_binding_free(ui_sprite_chipmunk_touch_binding_t binding) {
    ui_sprite_chipmunk_touch_responser_t responser = binding->m_responser;
    ui_sprite_chipmunk_obj_t obj = responser->m_obj;
    ui_sprite_chipmunk_env_t env = obj->m_env;
    ui_sprite_chipmunk_module_t module = env->m_module;

    TAILQ_REMOVE(&responser->m_bindings, binding, m_next_for_responser);
    TAILQ_REMOVE(&binding->m_trace->m_bindings, binding, m_next_for_trace);
    responser->m_binding_count--;

    if (TAILQ_EMPTY(&responser->m_bindings)) {
        responser->m_is_cur_processed = 0;
    }
    
    mem_free(module->m_alloc, binding);
}

ui_sprite_chipmunk_touch_binding_t
ui_sprite_chipmunk_touch_binding_find(ui_sprite_chipmunk_touch_responser_t responser, ui_sprite_chipmunk_touch_trace_t trace) {
    ui_sprite_chipmunk_touch_binding_t binding;

    TAILQ_FOREACH(binding, &responser->m_bindings, m_next_for_responser) {
        if (binding->m_trace == trace) return binding;
    }

    return NULL;
}

