#include <assert.h>
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite/ui_sprite_component.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui_sprite_chipmunk_touch_responser_i.h"
#include "ui_sprite_chipmunk_touch_binding_i.h"
#include "ui_sprite_chipmunk_touch_trace_i.h"

int ui_sprite_chipmunk_touch_responser_set_is_capture(ui_sprite_chipmunk_touch_responser_t responser, uint8_t is_capture) {
    responser->m_is_capture = is_capture;
    return 0;
}

int ui_sprite_chipmunk_touch_responser_set_is_grab(ui_sprite_chipmunk_touch_responser_t responser, uint8_t is_grab) {
    responser->m_is_grab = is_grab;
    return 0;
}

int ui_sprite_chipmunk_touch_responser_set_finger_count(ui_sprite_chipmunk_touch_responser_t responser, uint8_t finger_count) {
    ui_sprite_chipmunk_env_t env = responser->m_obj->m_env;

    if (finger_count <= 0 || finger_count > env->m_max_finger_count) {
        CPE_ERROR(env->m_module->m_em, "set finger count %d error!", finger_count);
        return -1;
    }

    if (ui_sprite_fsm_action_is_active(ui_sprite_fsm_action_from_data(responser))) {
        CPE_ERROR(env->m_module->m_em, "can`t set det finger count in active!");
        return -1;
    }

    responser->m_finger_count = finger_count;
    return 0;
}

void ui_sprite_chipmunk_touch_responser_cancel(ui_sprite_chipmunk_touch_responser_t responser) {
    ui_sprite_chipmunk_obj_t obj = responser->m_obj;
    ui_sprite_chipmunk_module_t module = responser->m_obj->m_env->m_module;
    ui_sprite_entity_t entity = ui_sprite_component_entity(ui_sprite_component_from_data(obj));

    assert(responser->m_binding_count <= responser->m_finger_count);

    while(!TAILQ_EMPTY(&responser->m_bindings)) {
        ui_sprite_chipmunk_touch_binding_free(TAILQ_FIRST(&responser->m_bindings));
    }
    assert(responser->m_binding_count == 0);

    if (responser->m_is_start) {
        if (ui_sprite_entity_debug(entity)) {
            CPE_INFO(
                module->m_em, "entity %d(%s): finger %d: chipmunk_touch cancel",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), responser->m_finger_count);
        }

        if (responser->m_on_cancel) responser->m_on_cancel(responser);

        responser->m_is_start = 0;
    }
}

void ui_sprite_chipmunk_touch_responser_init(
    ui_sprite_chipmunk_touch_responser_t responser,
    ui_sprite_entity_t entity, ui_sprite_chipmunk_obj_t obj)
{
    responser->m_obj = obj;
    responser->m_finger_count = 0;
    responser->m_is_capture = 0;
    responser->m_is_grab = 0;
    responser->m_is_start = 0;
    responser->m_threshold = obj->m_env->m_dft_threshold;
    responser->m_binding_count = 0;
    responser->m_is_active = 0;
    responser->m_is_cur_processed = 0;
    TAILQ_INIT(&responser->m_bindings);
}

void ui_sprite_chipmunk_touch_responser_fini(ui_sprite_chipmunk_touch_responser_t responser) {
    assert(responser->m_binding_count == 0);
    assert(responser->m_is_active == 0);

    while(!TAILQ_EMPTY(&responser->m_bindings)) {
        ui_sprite_chipmunk_touch_binding_free(TAILQ_FIRST(&responser->m_bindings));
    }
}

int ui_sprite_chipmunk_touch_responser_enter(ui_sprite_chipmunk_touch_responser_t responser) {
    assert(responser->m_finger_count > 0);
    assert(responser->m_binding_count == 0);
    assert(responser->m_is_start == 0);
    assert(responser->m_is_active == 0);

    TAILQ_INSERT_TAIL(&responser->m_obj->m_responsers, responser, m_next_for_obj);
    responser->m_is_active = 1;
    
    return 0;
}

void ui_sprite_chipmunk_touch_responser_exit(ui_sprite_chipmunk_touch_responser_t responser) {
    ui_sprite_chipmunk_env_t env = responser->m_obj->m_env;

    assert(responser->m_is_active == 1);
    
    ui_sprite_chipmunk_touch_responser_cancel(responser);

    assert(responser->m_binding_count < responser->m_finger_count);
    assert(responser->m_is_start == 0);
    assert(responser->m_finger_count > 0);
    assert(responser->m_binding_count == 0);

    while(!TAILQ_EMPTY(&responser->m_bindings)) {
        ui_sprite_chipmunk_touch_binding_free(TAILQ_FIRST(&responser->m_bindings));
    }
    
    TAILQ_REMOVE(&responser->m_obj->m_responsers, responser, m_next_for_obj);
    responser->m_is_active = 0;
}

static void ui_sprite_chipmunk_touch_responser_check_start(
    ui_sprite_chipmunk_module_t module, ui_sprite_entity_t entity,
    ui_sprite_chipmunk_obj_t obj, ui_sprite_chipmunk_touch_responser_t responser)
{
    assert(!responser->m_is_start);

    if (responser->m_threshold <= 0) {
        if (ui_sprite_entity_debug(entity)) {
            CPE_INFO(
                module->m_em, "entity %d(%s): finger %d: chipmunk_touch begin",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), responser->m_finger_count);
        }

        if (responser->m_on_begin) responser->m_on_begin(responser);

        responser->m_is_start = 1;
    }
    else {
        ui_sprite_chipmunk_touch_binding_t binding;
        TAILQ_FOREACH(binding, &responser->m_bindings, m_next_for_responser) {
            float distance =
                fabs(binding->m_cur_world_pt.x - binding->m_start_world_pt.x)
                + fabs(binding->m_cur_world_pt.y - binding->m_start_world_pt.y);
            if (distance < responser->m_threshold) {
                if (ui_sprite_entity_debug(entity)) {
                    CPE_INFO(
                        module->m_em, "entity %d(%s): finger %d: chipmunk_touch not active",
                        ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), responser->m_finger_count);
                }

                return;
            }
        }

        if (ui_sprite_entity_debug(entity)) {
            CPE_INFO(
                module->m_em, "entity %d(%s): finger %d: (threshold=%d) chipmunk_touch begin",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), responser->m_finger_count,
                responser->m_threshold);
        }

        if (responser->m_on_begin) responser->m_on_begin(responser);

        responser->m_is_start = 1;
    }
}

void ui_sprite_chipmunk_touch_responser_on_begin(ui_sprite_chipmunk_touch_responser_t responser) {
    ui_sprite_chipmunk_obj_t obj;
    ui_sprite_chipmunk_module_t module;
    ui_sprite_entity_t entity;

    if (responser->m_binding_count < responser->m_finger_count) return;

    if (!responser->m_is_start) {
        obj = responser->m_obj;
        module = obj->m_env->m_module;
        entity = ui_sprite_component_entity(ui_sprite_component_from_data(obj));

        ui_sprite_chipmunk_touch_responser_check_start(module, entity, obj, responser);
    }
}

void ui_sprite_chipmunk_touch_responser_on_move(ui_sprite_chipmunk_touch_responser_t responser) {
    ui_sprite_chipmunk_obj_t obj;
    ui_sprite_chipmunk_module_t module;
    ui_sprite_entity_t entity;

    if (responser->m_binding_count < responser->m_finger_count) return;

    obj = responser->m_obj;
    module = obj->m_env->m_module;
    entity = ui_sprite_component_entity(ui_sprite_component_from_data(obj));

    if (!responser->m_is_start) {
        ui_sprite_chipmunk_touch_responser_check_start(module, entity, obj, responser);
    }

    if (responser->m_is_start) {
        if (responser->m_on_move) responser->m_on_move(responser);
    }
}

void ui_sprite_chipmunk_touch_responser_on_end(ui_sprite_chipmunk_touch_responser_t responser) {
    ui_sprite_chipmunk_obj_t obj;
    ui_sprite_chipmunk_module_t module;
    ui_sprite_entity_t entity;

    if (!responser->m_is_start) return;
    if (responser->m_binding_count >= responser->m_finger_count) return;

    obj = responser->m_obj;
    module = obj->m_env->m_module;
    entity = ui_sprite_component_entity(ui_sprite_component_from_data(obj));

    if (ui_sprite_entity_debug(entity)) {
        CPE_INFO(
            module->m_em, "entity %d(%s): finger %d: touch end",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), responser->m_finger_count);
    }

    if (responser->m_on_end) responser->m_on_end(responser);

    responser->m_is_start = 0;
}
