#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/string_utils.h"
#include "cpe/timer/timer_manage.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dr/dr_json.h"
#include "ui/sprite/ui_sprite_world.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite/ui_sprite_component.h"
#include "ui/sprite/ui_sprite_event.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_calc.h"
#include "ui_sprite_fsm_ins_action_i.h"
#include "ui_sprite_fsm_ins_convertor_i.h"
#include "ui_sprite_fsm_ins_event_binding_i.h"
#include "ui_sprite_fsm_ins_attr_binding_i.h"
#include "ui_sprite_fsm_action_meta_i.h"

static ui_sprite_fsm_action_t
ui_sprite_fsm_action_create_i(ui_sprite_fsm_state_t fsm_state, const char * name, ui_sprite_fsm_action_meta_t meta) {
    ui_sprite_fsm_module_t module = fsm_state->m_ins->m_module;
    ui_sprite_fsm_action_t fsm_action;
    size_t name_len = strlen(name) + 1;

    if (ui_sprite_fsm_action_find_by_name(fsm_state, name) != NULL) {
        CPE_ERROR(module->m_em, "fsm create action %s: name duplicate", name);
        return NULL;
    }

    fsm_action = mem_alloc(module->m_alloc, sizeof(struct ui_sprite_fsm_action) + meta->m_size + name_len);
    if (fsm_action == NULL) {
        CPE_ERROR(module->m_em, "fsm create action %s: alloc fail", name);
        return NULL;
    }

    memcpy((char *)(fsm_action + 1) + meta->m_size, name, name_len);

    fsm_action->m_state = fsm_state;
    fsm_action->m_meta = meta;
    fsm_action->m_name = ((char *)(fsm_action + 1)) + meta->m_size;
    fsm_action->m_work = NULL;
    fsm_action->m_life_circle = ui_sprite_fsm_action_life_circle_working;
    fsm_action->m_duration = -1;
    fsm_action->m_duration_calc = NULL;
    fsm_action->m_condition = NULL;
    fsm_action->m_runing_time = 0.0f;
    fsm_action->m_apply_enter_evt = 0;
    fsm_action->m_is_update = 0;
    fsm_action->m_follow_to = NULL;
    fsm_action->m_runing_state = ui_sprite_fsm_action_state_deactive;
    fsm_action->m_addition_event = NULL;

    TAILQ_INIT(&fsm_action->m_convertors);
    TAILQ_INIT(&fsm_action->m_followers);
    TAILQ_INIT(&fsm_action->m_event_bindings);
    TAILQ_INIT(&fsm_action->m_attr_bindings);

    TAILQ_INSERT_TAIL(&fsm_state->m_actions, fsm_action, m_next_for_state);

    return fsm_action;
}

static void ui_sprite_fsm_action_free_i(ui_sprite_fsm_action_t fsm_action) {
    ui_sprite_fsm_state_t fsm_state = fsm_action->m_state;
    ui_sprite_fsm_module_t module = fsm_state->m_ins->m_module;

    if (fsm_action->m_runing_state == ui_sprite_fsm_action_state_runing) {
        ui_sprite_fsm_action_exit(fsm_action);
        assert(fsm_action->m_runing_state != ui_sprite_fsm_action_state_runing);
    }
    ui_sprite_fsm_action_set_runing_state(fsm_action, ui_sprite_fsm_action_state_deactive);

    assert(fsm_action->m_is_update == 0);
    assert(TAILQ_EMPTY(&fsm_action->m_event_bindings));
    assert(TAILQ_EMPTY(&fsm_action->m_attr_bindings));

    ui_sprite_fsm_convertor_free_all(fsm_action);

    while(!TAILQ_EMPTY(&fsm_action->m_followers)) {
        ui_sprite_fsm_action_set_follow_to(TAILQ_FIRST(&fsm_action->m_followers), NULL);
    }

    if (fsm_action->m_follow_to) {
        ui_sprite_fsm_action_set_follow_to(fsm_action, NULL);
    }

    TAILQ_REMOVE(&fsm_action->m_state->m_actions, fsm_action, m_next_for_state);

    if (fsm_action->m_work) mem_free(module->m_alloc, fsm_action->m_work);
    if (fsm_action->m_duration_calc) mem_free(module->m_alloc, fsm_action->m_duration_calc);
    if (fsm_action->m_condition) mem_free(module->m_alloc, fsm_action->m_condition);

    mem_free(module->m_alloc, fsm_action);
}

ui_sprite_fsm_action_t ui_sprite_fsm_action_create(ui_sprite_fsm_state_t fsm_state, const char * name, const char * type_name) {
    ui_sprite_fsm_module_t module = fsm_state->m_ins->m_module;
    ui_sprite_fsm_action_meta_t action_meta;
    ui_sprite_fsm_action_t fsm_action;

    action_meta = ui_sprite_fsm_action_meta_find(module, type_name);
    if (action_meta == NULL) {
        CPE_ERROR(module->m_em, "create action %s: action type %s not exist!", name, type_name);
        return NULL;
    }

    fsm_action = ui_sprite_fsm_action_create_i(fsm_state, name, action_meta);
    if (fsm_action == NULL) return NULL;

    if (action_meta->m_init_fun) {
        if (action_meta->m_init_fun(fsm_action, action_meta->m_init_fun_ctx) != 0) {
            CPE_ERROR(module->m_em, "create action %s: action type %s init fail!", name, type_name);
            ui_sprite_fsm_action_free_i(fsm_action);
            return NULL;
        }
    }
    else {
        bzero(ui_sprite_fsm_action_data(fsm_action), fsm_action->m_meta->m_size);
    }

    return fsm_action;
}

ui_sprite_fsm_action_t ui_sprite_fsm_action_clone(ui_sprite_fsm_state_t fsm_state, ui_sprite_fsm_action_t from) {
    ui_sprite_fsm_module_t module = fsm_state->m_ins->m_module;
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_create_i(fsm_state, from->m_name, from->m_meta);
    ui_sprite_fsm_convertor_t from_convertor;

    if (fsm_action == NULL) return NULL;

    fsm_action->m_life_circle = from->m_life_circle;
    fsm_action->m_duration = from->m_duration;
    fsm_action->m_apply_enter_evt = from->m_apply_enter_evt;

    if (from->m_work) {
        fsm_action->m_work = cpe_str_mem_dup(module->m_alloc, from->m_work);
        if (fsm_action->m_work == NULL) {
            CPE_ERROR(module->m_em, "clone action %s: copy enter fail!", from->m_name);
            ui_sprite_fsm_action_free_i(fsm_action);
            return NULL;
        }
    }

    if (from->m_condition) {
        fsm_action->m_condition = cpe_str_mem_dup(module->m_alloc, from->m_condition);
        if (fsm_action->m_condition == NULL) {
            CPE_ERROR(module->m_em, "clone action %s: copy condition fail!", from->m_name);
            ui_sprite_fsm_action_free_i(fsm_action);
            return NULL;
        }
    }

    if (from->m_duration_calc) {
        fsm_action->m_duration_calc = cpe_str_mem_dup(module->m_alloc, from->m_duration_calc);
        if (fsm_action->m_duration_calc == NULL) {
            CPE_ERROR(module->m_em, "clone action %s: copy condition fail!", from->m_name);
            ui_sprite_fsm_action_free_i(fsm_action);
            return NULL;
        }
    }

    if (from->m_follow_to) {
        if (ui_sprite_fsm_action_set_follow_to(fsm_action, from->m_follow_to->m_name) != 0) {
            CPE_ERROR(module->m_em, "clone action %s: set flolow-to %s fail!", from->m_name, from->m_follow_to->m_name);
            ui_sprite_fsm_action_free_i(fsm_action);
            return NULL;
        }
    }

    if (from->m_meta->m_copy_fun) {
        if (from->m_meta->m_copy_fun(fsm_action, from, from->m_meta->m_copy_fun_ctx) != 0) {
            CPE_ERROR(module->m_em, "clone action %s: action type %s copy fail!", from->m_name, from->m_meta->m_name);
            ui_sprite_fsm_action_free_i(fsm_action);
            return NULL;
        }
    }
    else {
        memcpy(ui_sprite_fsm_action_data(fsm_action), ui_sprite_fsm_action_data(from), fsm_action->m_meta->m_size);
    }

    TAILQ_FOREACH(from_convertor, &from->m_convertors, m_next_for_action) {
        if (ui_sprite_fsm_convertor_clone(fsm_action, from_convertor) == NULL) {
            CPE_ERROR(module->m_em, "clone action %s: convertor %s copy fail!", from->m_name, from_convertor->m_event);
            ui_sprite_fsm_action_free_i(fsm_action);
            return NULL;
        }
    }

    return fsm_action;
}

void ui_sprite_fsm_action_free(ui_sprite_fsm_action_t fsm_action) {
    if (fsm_action->m_meta->m_free_fun) fsm_action->m_meta->m_free_fun(fsm_action, fsm_action->m_meta->m_free_fun_ctx);
    ui_sprite_fsm_action_free_i(fsm_action);
}

ui_sprite_fsm_action_t ui_sprite_fsm_action_find(ui_sprite_fsm_state_t fsm_state, const char * name) {
    ui_sprite_fsm_action_t action;

    TAILQ_FOREACH(action, &fsm_state->m_actions, m_next_for_state) {
        if (strcmp(action->m_name, name) == 0) return action;
    }

    return NULL;
}

int ui_sprite_fsm_action_set_follow_to(ui_sprite_fsm_action_t fsm_action, const char * follow_to) {
    ui_sprite_fsm_module_t module = fsm_action->m_state->m_ins->m_module;
    ui_sprite_fsm_action_t follow_to_action = NULL;

    if (follow_to) {
        follow_to_action = ui_sprite_fsm_action_find(fsm_action->m_state, follow_to);
        if (follow_to_action == NULL) {
            CPE_ERROR(module->m_em, "action %s set follow to %s: follow to action not exist", fsm_action->m_name, follow_to);
            return -1;
        }
    }

    if (fsm_action->m_follow_to) {
        TAILQ_REMOVE(&fsm_action->m_follow_to->m_followers, fsm_action, m_next_for_follow);
    }

    fsm_action->m_follow_to = follow_to_action;
    if (fsm_action->m_follow_to) {
        TAILQ_INSERT_TAIL(&fsm_action->m_follow_to->m_followers, fsm_action, m_next_for_follow);
    }

    return 0;
}

const char * ui_sprite_fsm_action_name(ui_sprite_fsm_action_t fsm_action) {
    return fsm_action->m_name;
}

const char * ui_sprite_fsm_action_type_name(ui_sprite_fsm_action_t fsm_action) {
    return fsm_action->m_meta->m_name;
}

float ui_sprite_fsm_action_runing_time(ui_sprite_fsm_action_t fsm_action) {
    return fsm_action->m_runing_time;
}

uint8_t ui_sprite_fsm_action_is_active(ui_sprite_fsm_action_t fsm_action) {
    return fsm_action->m_runing_state == ui_sprite_fsm_action_state_runing;
}

uint8_t ui_sprite_fsm_action_apply_enter_evt(ui_sprite_fsm_action_t fsm_action) {
    return fsm_action->m_apply_enter_evt;
}

void ui_sprite_fsm_action_set_apply_enter_evt(ui_sprite_fsm_action_t fsm_action, uint8_t apply_enter_evt) {
    fsm_action->m_apply_enter_evt = apply_enter_evt;
}

ui_sprite_fsm_action_life_circle_t ui_sprite_fsm_action_life_circle(ui_sprite_fsm_action_t fsm_action) {
    return fsm_action->m_life_circle;
}

int ui_sprite_fsm_action_set_life_circle(ui_sprite_fsm_action_t fsm_action, ui_sprite_fsm_action_life_circle_t  life_circle) {
    if (fsm_action->m_runing_state != ui_sprite_fsm_action_state_deactive) {
        CPE_ERROR(fsm_action->m_state->m_ins->m_module->m_em, "can`t set action life circle in active!");
        return -1;
    }

    fsm_action->m_life_circle = life_circle;
    return 0;
}

const char * ui_sprite_fsm_action_work(ui_sprite_fsm_action_t fsm_action) {
    return fsm_action->m_work;
}

int ui_sprite_fsm_action_set_work(ui_sprite_fsm_action_t fsm_action, const char * enter) {
    ui_sprite_fsm_module_t module = fsm_action->m_state->m_ins->m_module;

    if (fsm_action->m_work) {
        mem_free(module->m_alloc, fsm_action->m_work);
        fsm_action->m_work = NULL;
    }

    if (enter) {
        fsm_action->m_work = cpe_str_mem_dup(module->m_alloc, enter);
        if (fsm_action->m_work == NULL) {
            CPE_ERROR(module->m_em, "copy enter fail");
            return -1;
        }
    }

    return 0;
}

float ui_sprite_fsm_action_duration(ui_sprite_fsm_action_t fsm_action) {
    return fsm_action->m_duration;
}

int ui_sprite_fsm_action_set_duration_calc(ui_sprite_fsm_action_t fsm_action, const char * duration) {
    if (fsm_action->m_life_circle != ui_sprite_fsm_action_life_circle_duration
        && fsm_action->m_life_circle != ui_sprite_fsm_action_life_circle_working)
    {
        CPE_ERROR(
            fsm_action->m_state->m_ins->m_module->m_em, "can`t set duration in life circle %d!",
            fsm_action->m_life_circle);
        return -1;
    }

    if (duration == NULL) {
        CPE_ERROR(fsm_action->m_state->m_ins->m_module->m_em, "duration str is null error!");
        return -1;
    }

    fsm_action->m_duration = 0.0F;
    
    if (fsm_action->m_duration_calc) {
        mem_free(fsm_action->m_state->m_ins->m_module->m_alloc, fsm_action->m_duration_calc);
    }

    fsm_action->m_duration_calc = cpe_str_mem_dup(fsm_action->m_state->m_ins->m_module->m_alloc, duration);
    if (fsm_action->m_duration_calc == NULL) {
        CPE_ERROR(fsm_action->m_state->m_ins->m_module->m_em, "duration str dup fail!");
        return -1;
    }

    return 0;
}

int ui_sprite_fsm_action_set_duration(ui_sprite_fsm_action_t fsm_action, float duration) {
    if (fsm_action->m_life_circle != ui_sprite_fsm_action_life_circle_duration
        && fsm_action->m_life_circle != ui_sprite_fsm_action_life_circle_working)
    {
        CPE_ERROR(
            fsm_action->m_state->m_ins->m_module->m_em, "can`t set duration in life circle %d!",
            fsm_action->m_life_circle);
        return -1;
    }

    if (duration <= 0.0f) {
        CPE_ERROR(fsm_action->m_state->m_ins->m_module->m_em, "duration %f error!", duration);
        return -1;
    }

    fsm_action->m_duration = duration;
    
    if (fsm_action->m_duration_calc) {
        mem_free(fsm_action->m_state->m_ins->m_module->m_alloc, fsm_action->m_duration_calc);
        fsm_action->m_duration_calc = NULL;
    }
    
    return 0;
}

int ui_sprite_fsm_action_set_condition(ui_sprite_fsm_action_t fsm_action, const char * condition) {
    ui_sprite_fsm_module_t module = fsm_action->m_state->m_ins->m_module;

    if (fsm_action->m_condition) {
        mem_free(module->m_alloc, fsm_action->m_condition);
        fsm_action->m_condition = NULL;
    }

    if (condition) {
        fsm_action->m_condition = cpe_str_mem_dup_trim(module->m_alloc, condition);
        if (fsm_action->m_condition == NULL) {
            CPE_ERROR(module->m_em, "copy condition fail");
            return -1;
        }
    }

    return 0;
}

ui_sprite_fsm_state_t ui_sprite_fsm_action_state(ui_sprite_fsm_action_t fsm_action) {
    return fsm_action->m_state;
}

ui_sprite_fsm_action_state_t ui_sprite_fsm_action_runing_state(ui_sprite_fsm_action_t fsm_action) {
    return fsm_action->m_runing_state;
}

ui_sprite_component_t ui_sprite_fsm_action_to_component(ui_sprite_fsm_action_t fsm_action) {
    return ui_sprite_fsm_to_component(fsm_action->m_state->m_ins);
}

ui_sprite_entity_t ui_sprite_fsm_action_to_entity(ui_sprite_fsm_action_t fsm_action) {
    return ui_sprite_fsm_to_entity(fsm_action->m_state->m_ins);
}

ui_sprite_world_t ui_sprite_fsm_action_to_world(ui_sprite_fsm_action_t fsm_action) {
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    return entity ? ui_sprite_entity_world(entity) : NULL;
}

ui_sprite_fsm_action_meta_t ui_sprite_fsm_action_meta(ui_sprite_fsm_action_t fsm_action) {
    return fsm_action->m_meta;
}

void * ui_sprite_fsm_action_data(ui_sprite_fsm_action_t fsm_action) {
    return fsm_action + 1;
}

size_t ui_sprite_fsm_action_data_size(ui_sprite_fsm_action_t fsm_action) {
    return fsm_action->m_meta->m_size;
}

ui_sprite_fsm_action_t ui_sprite_fsm_action_from_data(void * data) {
    return ((ui_sprite_fsm_action_t)data) - 1;
}

uint8_t ui_sprite_fsm_action_is_update(ui_sprite_fsm_action_t fsm_action) {
    return fsm_action->m_is_update;
}

void ui_sprite_fsm_action_send_event(ui_sprite_fsm_action_t fsm_action, LPDRMETA meta, void const * data, size_t size) {
    ui_sprite_component_send_event(ui_sprite_fsm_action_to_component(fsm_action), meta, data, size);
}

void ui_sprite_fsm_action_send_event_to(
    ui_sprite_fsm_action_t fsm_action, const char * target,
    LPDRMETA meta, void const * data, size_t size)
{
    ui_sprite_component_send_event_to(ui_sprite_fsm_action_to_component(fsm_action), target, meta, data, size);
}

int ui_sprite_fsm_action_start_update(ui_sprite_fsm_action_t fsm_action) {
    ui_sprite_fsm_state_t fsm_state = fsm_action->m_state;
    ui_sprite_fsm_module_t module = fsm_state->m_ins->m_module;

    if (fsm_action->m_runing_state != ui_sprite_fsm_action_state_runing) {
        CPE_ERROR(
            module->m_em, "action %s start update: action not start!!",
            fsm_action->m_name);
        return -1;
    }

    if (fsm_action->m_is_update) return 0;

    if (fsm_action->m_meta->m_update_fun == NULL) {
        CPE_ERROR(
            module->m_em, "action %s start update: not support update!",
            fsm_action->m_name);
        return -1;
    }

    TAILQ_INSERT_TAIL(&fsm_state->m_updating_actions, fsm_action, m_next_for_update);
    fsm_action->m_is_update = 1;

    return 0;
}

void ui_sprite_fsm_action_stop_update(ui_sprite_fsm_action_t fsm_action) {
    ui_sprite_fsm_state_t fsm_state = fsm_action->m_state;

    if (!fsm_action->m_is_update) return;

    TAILQ_REMOVE(&fsm_state->m_updating_actions, fsm_action, m_next_for_update);
    fsm_action->m_is_update = 0;
}

void ui_sprite_fsm_action_sync_update(ui_sprite_fsm_action_t fsm_action, uint8_t is_start) {
    if (is_start) {
        if (!fsm_action->m_is_update) {
            ui_sprite_fsm_action_start_update(fsm_action);
        }
    }
    else {
        if (fsm_action->m_is_update) {
            ui_sprite_fsm_action_stop_update(fsm_action);
        }
    }
}

int ui_sprite_fsm_action_enter(ui_sprite_fsm_action_t fsm_action) {
    ui_sprite_fsm_module_t module = fsm_action->m_state->m_ins->m_module;
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_fsm_ins_t fsm = fsm_action->m_state->m_ins;

    if (fsm_action->m_state != fsm->m_cur_state) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): %s: action %s(%s) enter: state %s not active",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), ui_sprite_fsm_ins_path(fsm),
            fsm_action->m_meta->m_name, fsm_action->m_name, fsm_action->m_state->m_name);
        return -1;
    }

    if (fsm_action->m_runing_state == ui_sprite_fsm_action_state_runing) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): %s: action %s(%s) is already entered",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), ui_sprite_fsm_ins_path(fsm),
            fsm_action->m_meta->m_name, fsm_action->m_name);
        return -1;
    }

    if (fsm_action->m_duration_calc) {
        if (ui_sprite_fsm_action_try_calc_float(&fsm_action->m_duration, fsm_action->m_duration_calc, fsm_action, NULL, module->m_em) != 0) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): %s: action %s(%s): calc duration %s fail",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), ui_sprite_fsm_ins_path(fsm),
                fsm_action->m_meta->m_name, fsm_action->m_name, fsm_action->m_duration_calc);
            return -1;
        }
    }

    if (ui_sprite_entity_debug(entity)) {
        CPE_INFO(
            module->m_em, "entity %d(%s): %s: action %s(%s) enter",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), ui_sprite_fsm_ins_path(fsm),
            fsm_action->m_meta->m_name, fsm_action->m_name);
    }

    ui_sprite_fsm_action_set_runing_state(fsm_action, ui_sprite_fsm_action_state_runing);
    fsm_action->m_runing_time = 0.0f;

    if (ui_sprite_fsm_convertor_enter_all(fsm_action) != 0) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): %s: action %s(%s) enter fail(create convertor fail!)",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), ui_sprite_fsm_ins_path(fsm),
            fsm_action->m_meta->m_name, fsm_action->m_name);
        goto UI_SPRITE_FSM_ACTION_ENTER_FAIL;
    }

    if (fsm_action->m_meta->m_enter_fun) {
        if (fsm_action->m_meta->m_enter_fun(fsm_action, fsm_action->m_meta->m_enter_fun_ctx) != 0) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): %s: action %s(%s) enter fail",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), ui_sprite_fsm_ins_path(fsm),
                fsm_action->m_meta->m_name, fsm_action->m_name);

            if (fsm_action->m_is_update) {
                ui_sprite_fsm_action_stop_update(fsm_action);
                assert(!fsm_action->m_is_update);
            }

            goto UI_SPRITE_FSM_ACTION_ENTER_FAIL;
        }
    }

    return 0;

UI_SPRITE_FSM_ACTION_ENTER_FAIL:
    ui_sprite_fsm_convertor_exit_all(fsm_action);
    ui_sprite_fsm_action_event_binding_free_all(fsm_action);
    ui_sprite_fsm_action_attr_binding_free_all(fsm_action);
    ui_sprite_fsm_action_set_runing_state(fsm_action, ui_sprite_fsm_action_state_done);
    fsm_action->m_runing_time = 0.0f;
    return -1;
}

void ui_sprite_fsm_action_exit(ui_sprite_fsm_action_t fsm_action) {
    ui_sprite_fsm_module_t module = fsm_action->m_state->m_ins->m_module;
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_fsm_ins_t fsm = fsm_action->m_state->m_ins;

    if (fsm_action->m_runing_state != ui_sprite_fsm_action_state_runing) return;

    if (ui_sprite_entity_debug(entity)) {
        CPE_INFO(
            module->m_em, "entity %d(%s): %s: action %s(%s) exit",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), ui_sprite_fsm_ins_path(fsm),
            fsm_action->m_meta->m_name, fsm_action->m_name);
    }

    if (fsm_action->m_is_update) {
        ui_sprite_fsm_action_stop_update(fsm_action);
        assert(!fsm_action->m_is_update);
    }

    if (fsm_action->m_meta->m_exit_fun) {
        fsm_action->m_meta->m_exit_fun(fsm_action, fsm_action->m_meta->m_exit_fun_ctx);
    }

    ui_sprite_fsm_convertor_exit_all(fsm_action);
    ui_sprite_fsm_action_event_binding_free_all(fsm_action);
    ui_sprite_fsm_action_attr_binding_free_all(fsm_action);

    fsm_action->m_runing_time = 0.0f;
    ui_sprite_fsm_action_set_runing_state(fsm_action, ui_sprite_fsm_action_state_done);
}

void ui_sprite_fsm_action_set_runing_state(ui_sprite_fsm_action_t fsm_action, ui_sprite_fsm_action_state_t runing_state) {
    ui_sprite_fsm_state_t fsm_state = fsm_action->m_state;

    switch(fsm_action->m_runing_state) {
    case ui_sprite_fsm_action_state_waiting:
        TAILQ_REMOVE(&fsm_state->m_waiting_actions, fsm_action, m_next_for_work);
        break;
    case ui_sprite_fsm_action_state_runing:
        TAILQ_REMOVE(&fsm_state->m_runing_actions, fsm_action, m_next_for_work);
        break;
    case ui_sprite_fsm_action_state_done:
        TAILQ_REMOVE(&fsm_state->m_done_actions, fsm_action, m_next_for_work);
        break;
    default:
        break;
    }

    fsm_action->m_runing_state = runing_state;

    switch(fsm_action->m_runing_state) {
    case ui_sprite_fsm_action_state_waiting:
        TAILQ_INSERT_TAIL(&fsm_state->m_waiting_actions, fsm_action, m_next_for_work);
        break;
    case ui_sprite_fsm_action_state_runing:
        TAILQ_INSERT_TAIL(&fsm_state->m_runing_actions, fsm_action, m_next_for_work);
        break;
    case ui_sprite_fsm_action_state_done:
        TAILQ_INSERT_TAIL(&fsm_state->m_done_actions, fsm_action, m_next_for_work);
        break;
    default:
        break;
    }
}

int ui_sprite_fsm_action_check_do_enter(ui_sprite_fsm_action_t fsm_action) {
    ui_sprite_fsm_module_t module = fsm_action->m_state->m_ins->m_module;
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_fsm_ins_t fsm = fsm_action->m_state->m_ins;

    if (fsm_action->m_work) {
        ui_sprite_fsm_action_event_binding_t binding;
        ui_sprite_event_t event;
        const char * event_name;
        uint16_t process_count = 0;

        event = ui_sprite_fsm_action_build_event(fsm_action, module->m_alloc, fsm_action->m_work, NULL);
        if (event == NULL) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): %s: action %s(%s) build work event '%s' fail",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), ui_sprite_fsm_ins_path(fsm),
                fsm_action->m_meta->m_name, fsm_action->m_name, fsm_action->m_work);
            return -1;
        }

        event_name = dr_meta_name(event->meta);

        TAILQ_FOREACH(binding, &fsm_action->m_event_bindings, m_next) {
            if (strcmp(ui_sprite_event_handler_process_event(binding->m_handler), event_name) == 0) {
                ++process_count;

                if (ui_sprite_entity_debug(entity)) {
                    struct mem_buffer buffer;
                    mem_buffer_init(&buffer, module->m_alloc);

                    CPE_INFO(
                        module->m_em, "entity %d(%s): %s: action %s(%s) send work event %s: %s",
                        ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), ui_sprite_fsm_ins_path(fsm),
                        fsm_action->m_meta->m_name, fsm_action->m_name, dr_meta_name(event->meta),
                        dr_json_dump_inline(&buffer, event->data, event->size, event->meta));
                    
                    mem_buffer_clear(&buffer);
                }

                (*ui_sprite_event_handler_process_fun(binding->m_handler))
                    (ui_sprite_event_handler_process_ctx(binding->m_handler), event);
            }
        }

        if (process_count == 0) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): %s: action %s(%s) no handler process work event %s",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), ui_sprite_fsm_ins_path(fsm),
                fsm_action->m_meta->m_name, fsm_action->m_name, dr_meta_name(event->meta));
            mem_free(module->m_alloc, event);
            return -1;
        }

        mem_free(module->m_alloc, event);

        return 0;
    }

    if (fsm_action->m_apply_enter_evt && fsm_action->m_state->m_enter_event) {
        ui_sprite_event_t event = fsm_action->m_state->m_enter_event;
        const char * event_name = dr_meta_name(event->meta);
        ui_sprite_fsm_action_event_binding_t binding;
        ui_sprite_fsm_convertor_t convertor;

        TAILQ_FOREACH(binding, &fsm_action->m_event_bindings, m_next) {
            if (strcmp(ui_sprite_event_handler_process_event(binding->m_handler), event_name) == 0) {
                if (ui_sprite_entity_debug(entity)) {
                    CPE_INFO(
                        module->m_em, "entity %d(%s): %s: action %s(%s) forward event %s: %s",
                        ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), ui_sprite_fsm_ins_path(fsm),
                        fsm_action->m_meta->m_name, fsm_action->m_name, dr_meta_name(event->meta),
                        dr_json_dump_inline(&module->m_dump_buffer, event->data, event->size, event->meta));
                }

                (*ui_sprite_event_handler_process_fun(binding->m_handler))
                    (ui_sprite_event_handler_process_ctx(binding->m_handler), event);
            }
        }

        TAILQ_FOREACH(convertor, &fsm_action->m_convertors, m_next_for_action) {
            if (strcmp(ui_sprite_event_handler_process_event(convertor->m_handler), event_name) == 0) {
                if (ui_sprite_entity_debug(entity)) {
                    CPE_INFO(
                        module->m_em, "entity %d(%s): %s: action %s(%s) forward event %s: %s",
                        ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), ui_sprite_fsm_ins_path(fsm),
                        fsm_action->m_meta->m_name, fsm_action->m_name, dr_meta_name(event->meta),
                        dr_json_dump_inline(&module->m_dump_buffer, event->data, event->size, event->meta));
                }

                (*ui_sprite_event_handler_process_fun(convertor->m_handler))
                    (ui_sprite_event_handler_process_ctx(convertor->m_handler), event);
            }
        }
    }

    return 0;
}

ui_sprite_fsm_action_t ui_sprite_fsm_action_find_by_name(ui_sprite_fsm_state_t fsm_state, const char * name) {
    ui_sprite_fsm_action_t action;

    if (name[0] == 0) return NULL;

    TAILQ_FOREACH(action, &fsm_state->m_actions, m_next_for_state) {
        if (strcmp(action->m_name, name) == 0) return action;
    }
    
    return NULL;
}

static ui_sprite_fsm_action_t ui_sprite_fsm_state_actions_next(ui_sprite_fsm_action_it_t it) {
    ui_sprite_fsm_action_t * data = (ui_sprite_fsm_action_t *)(it->m_data);
    ui_sprite_fsm_action_t r;

    if (*data == NULL) return NULL;

    r = *data;

    *data = TAILQ_NEXT(r, m_next_for_state);

    return r;
}

void ui_sprite_fsm_state_actions(ui_sprite_fsm_action_it_t it, ui_sprite_fsm_state_t state) {
    *(ui_sprite_fsm_action_t *)(it->m_data) = TAILQ_FIRST(&state->m_actions);
    it->next = ui_sprite_fsm_state_actions_next;
}
