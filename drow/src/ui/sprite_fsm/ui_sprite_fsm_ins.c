#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/stream_buffer.h"
#include "ui/sprite/ui_sprite_component.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui_sprite_fsm_ins_i.h"
#include "ui_sprite_fsm_ins_state_i.h"
#include "ui_sprite_fsm_ins_action_i.h"
#include "ui_sprite_fsm_action_meta_i.h"

void ui_sprite_fsm_ins_init(ui_sprite_fsm_ins_t fsm, ui_sprite_fsm_module_t module, ui_sprite_fsm_ins_t parent_fsm) {
    bzero(fsm, sizeof(*fsm));
    fsm->m_module = module;
    fsm->m_parent = parent_fsm;
    TAILQ_INIT(&fsm->m_childs);
    TAILQ_INIT(&fsm->m_states);
}

void ui_sprite_fsm_ins_fini(ui_sprite_fsm_ins_t fsm) {
    if (fsm->m_cur_state) {
        ui_sprite_fsm_ins_exit(fsm);
        assert(fsm->m_cur_state == NULL);
    }

    while(!TAILQ_EMPTY(&fsm->m_states)) {
        ui_sprite_fsm_state_free(TAILQ_FIRST(&fsm->m_states));
    }

    assert(TAILQ_EMPTY(&fsm->m_childs));

    assert(fsm->m_cur_state == NULL);
    assert(fsm->m_init_state == NULL);
    assert(fsm->m_init_call_state == NULL);
}

void ui_sprite_fsm_ins_reinit(ui_sprite_fsm_ins_t fsm) {
    ui_sprite_fsm_ins_fini(fsm);
}

int ui_sprite_fsm_ins_copy(ui_sprite_fsm_ins_t to, ui_sprite_fsm_ins_t from) {
    ui_sprite_fsm_state_t from_fsm_state;

    TAILQ_FOREACH(from_fsm_state, &from->m_states, m_next_for_ins) {
        ui_sprite_fsm_state_t state = ui_sprite_fsm_state_clone(to, from_fsm_state);
        if (state == NULL) return -1;
    }

    if (from->m_init_state) {
        to->m_init_state = ui_sprite_fsm_state_find_by_name(to, from->m_init_state->m_name);
    }

    if (from->m_init_call_state) {
        to->m_init_call_state = ui_sprite_fsm_state_find_by_name(to, from->m_init_call_state->m_name);
    }

    return 0;
}

ui_sprite_component_t ui_sprite_fsm_to_component(ui_sprite_fsm_ins_t fsm) {
    while(fsm->m_parent) {
        fsm = fsm->m_parent;
    }

    return ui_sprite_component_from_data(fsm);
}

ui_sprite_entity_t ui_sprite_fsm_to_entity(ui_sprite_fsm_ins_t fsm) {
    ui_sprite_component_t component = ui_sprite_fsm_to_component(fsm);

    return component ? ui_sprite_component_entity(component) : NULL;
}

ui_sprite_world_t ui_sprite_fsm_to_world(ui_sprite_fsm_ins_t fsm) {
    ui_sprite_entity_t entity = ui_sprite_fsm_to_entity(fsm);
    return entity ? ui_sprite_entity_world(entity) : NULL;
}

ui_sprite_fsm_ins_t ui_sprite_fsm_parent(ui_sprite_fsm_ins_t fsm) {
    return fsm->m_parent;
}

int ui_sprite_fsm_set_default_state(ui_sprite_fsm_ins_t fsm, const char * state_name) {
    ui_sprite_fsm_module_t module = fsm->m_module;
    ui_sprite_fsm_state_t fsm_state = ui_sprite_fsm_state_find_by_name(fsm, state_name);

    if (fsm_state == NULL) {
        CPE_ERROR(module->m_em, "ui_sprite_fsm_set_default_state: state %s not exist!", state_name);
        return -1;
    }

    fsm->m_init_state = fsm_state;

    return 0;
}

ui_sprite_fsm_state_t ui_sprite_fsm_default_state(ui_sprite_fsm_ins_t fsm) {
    return fsm->m_init_state;
}

int ui_sprite_fsm_set_default_call_state(ui_sprite_fsm_ins_t fsm, const char * state_name) {
    ui_sprite_fsm_module_t module = fsm->m_module;
    
    if (state_name) {
        ui_sprite_fsm_state_t fsm_state = ui_sprite_fsm_state_find_by_name(fsm, state_name);

        if (fsm_state == NULL) {
            CPE_ERROR(module->m_em, "ui_sprite_fsm_set_default_call_state: state %s not exist!", state_name);
            return -1;
        }

        fsm->m_init_call_state = fsm_state;
    }
    else {
        fsm->m_init_call_state = NULL;
    }

    return 0;
}

ui_sprite_fsm_state_t ui_sprite_fsm_default_call_state(ui_sprite_fsm_ins_t fsm) {
    return fsm->m_init_call_state;
}

ui_sprite_fsm_state_t ui_sprite_fsm_current_state(ui_sprite_fsm_ins_t fsm) {
    return fsm->m_cur_state;
}

int ui_sprite_fsm_is_in_state(ui_sprite_fsm_ins_t fsm, const char * state_path) {
    ui_sprite_fsm_ins_t child_fsm;
    const char * sep;
    size_t len;

    if (fsm->m_cur_state == NULL) return 0;

    sep = strchr(state_path, '.');

    if (sep == NULL) return strcmp(fsm->m_cur_state->m_name, state_path) == 0;

    len = sep - state_path;
    if (memcmp(fsm->m_cur_state->m_name, state_path, len) != 0
        || fsm->m_cur_state->m_name[len] != 0)
    {
        return 0;
    }

    TAILQ_FOREACH(child_fsm, &fsm->m_childs, m_next_for_parent) {
        if (child_fsm->m_cur_state == NULL) continue;
        if (ui_sprite_fsm_is_in_state(child_fsm, sep + 1)) return 1;
    }

    return 0;
}

int ui_sprite_fsm_have_state(ui_sprite_fsm_ins_t fsm, const char * state_path) {
    ui_sprite_fsm_ins_t child_fsm;
    const char * sep;
    size_t len;
    ui_sprite_fsm_state_t state;
    
    if (fsm->m_cur_state == NULL) return 0;

    sep = strchr(state_path, '.');

    if (sep == NULL) return ui_sprite_fsm_state_find_by_name(fsm, state_path) ? 1 : 0;

    len = sep - state_path;

    TAILQ_FOREACH(state, &fsm->m_states, m_next_for_ins) {
        if (memcmp(state->m_name, state_path, len) == 0 && state->m_name[len] == 0) {
            break;
        }
    }
    if (state == NULL) return 0;
    
    TAILQ_FOREACH(child_fsm, &fsm->m_childs, m_next_for_parent) {
        ui_sprite_fsm_action_t action = ui_sprite_fsm_action_from_data(child_fsm);
        if (action->m_state != state) continue;
        
        if (ui_sprite_fsm_have_state(child_fsm, sep + 1)) return 1;
    }

    return 0;
}

int ui_sprite_fsm_ins_enter(ui_sprite_fsm_ins_t fsm) {
    ui_sprite_entity_t entity = ui_sprite_fsm_to_entity(fsm);
    ui_sprite_fsm_state_t state = NULL;

    if (fsm->m_cur_state) {
        CPE_ERROR(
            fsm->m_module->m_em, "entity %d(%s): %s: enter: fsm is already enterd!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity),
            ui_sprite_fsm_ins_path(fsm));
        return -1;
    }

    if (ui_sprite_entity_debug(entity)) {
        CPE_INFO(
            fsm->m_module->m_em, "entity %d(%s): %s: enter!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity),
            ui_sprite_fsm_ins_path(fsm));
    }

    if (fsm->m_init_call_state) {
        state = fsm->m_init_call_state;
        state->m_return_to = fsm->m_init_state;
    }
    else if (fsm->m_init_state) {
        state = fsm->m_init_state;
    }

    if (state == NULL) {
        CPE_ERROR(
            fsm->m_module->m_em, "entity %d(%s): fsm enter: no current state!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }

    if (ui_sprite_fsm_state_enter(state) != 0) {
        CPE_ERROR(
            fsm->m_module->m_em, "entity %d(%s): fsm enter: state %s enter fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), state->m_name);
        state->m_return_to = NULL;
        assert(fsm->m_cur_state == NULL);
        return -1;
    }

    if (fsm->m_parent) {
        TAILQ_INSERT_TAIL(&fsm->m_parent->m_childs, fsm, m_next_for_parent);
    }

    assert(fsm->m_cur_state == state);

    return 0;
}

int ui_sprite_fsm_ins_set_state(ui_sprite_fsm_ins_t fsm, const char * switch_to, const char * call) {
    ui_sprite_entity_t entity = ui_sprite_fsm_to_entity(fsm);
    ui_sprite_fsm_state_t switch_to_state = ui_sprite_fsm_state_find_by_name(fsm, switch_to);
    ui_sprite_fsm_state_t call_state = call ? ui_sprite_fsm_state_find_by_name(fsm, call) : NULL;
    ui_sprite_fsm_state_t enter_state;

    if (switch_to_state == NULL) {
        CPE_ERROR(
            fsm->m_module->m_em, "entity %d(%s): %s: set state: switch to state %s not exist!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity),
            ui_sprite_fsm_ins_path(fsm), switch_to);
        return -1;
    }

    if (call && call_state == NULL) {
        CPE_ERROR(
            fsm->m_module->m_em, "entity %d(%s): %s: set state: call state %s not exist!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity),
            ui_sprite_fsm_ins_path(fsm), call);
        return -1;
    }

    if (fsm->m_cur_state) {
        if (ui_sprite_entity_debug(entity)) {
            CPE_INFO(
                fsm->m_module->m_em, "entity %d(%s): %s: set state: exit old state!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity),
                ui_sprite_fsm_ins_path(fsm));
        }

        ui_sprite_fsm_state_exit(fsm->m_cur_state);
        assert(fsm->m_cur_state == NULL);

        if (fsm->m_parent) {
            TAILQ_REMOVE(&fsm->m_parent->m_childs, fsm, m_next_for_parent);
        }
    }

    if (call_state) {
        enter_state = call_state;
        enter_state->m_return_to = switch_to_state;
    }
    else {
        assert(switch_to_state);
        enter_state = switch_to_state;
    }

    assert(enter_state);

    if (ui_sprite_fsm_state_enter(enter_state) != 0) {
        CPE_ERROR(
            fsm->m_module->m_em, "entity %d(%s): fsm enter: switch: state %s enter fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), enter_state->m_name);
        enter_state->m_return_to = NULL;
        assert(fsm->m_cur_state == NULL);
        return -1;
    }

    assert(fsm->m_cur_state == enter_state);

    if (ui_sprite_entity_debug(entity)) {
        CPE_INFO(
            fsm->m_module->m_em, "entity %d(%s): %s: set state: enter state %s success",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity),
            ui_sprite_fsm_ins_path(fsm), enter_state->m_name);
    }

    return 0;
}

void ui_sprite_fsm_ins_exit(ui_sprite_fsm_ins_t fsm) {
    ui_sprite_entity_t entity = ui_sprite_fsm_to_entity(fsm);

    if (fsm->m_cur_state == NULL) return;

    if (ui_sprite_entity_debug(entity)) {
        CPE_INFO(
            fsm->m_module->m_em, "entity %d(%s): %s: exit!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity),
            ui_sprite_fsm_ins_path(fsm));
    }

    ui_sprite_fsm_state_exit(fsm->m_cur_state);
    assert(fsm->m_cur_state == NULL);

    if (fsm->m_parent) {
        TAILQ_REMOVE(&fsm->m_parent->m_childs, fsm, m_next_for_parent);
    }
}

void ui_sprite_fsm_ins_check(ui_sprite_fsm_ins_t fsm) {
    uint16_t runing_action_count;

    assert(fsm->m_cur_state);

INS_CHECK_AGAIN:
    runing_action_count = ui_sprite_fsm_state_check_actions(fsm->m_cur_state);
    if (runing_action_count > 0) return;

    ui_sprite_fsm_state_process_complete(fsm->m_cur_state);
    if (fsm->m_cur_state) goto INS_CHECK_AGAIN;
}

void ui_sprite_fsm_ins_update(ui_sprite_fsm_ins_t fsm, float delta) {
    if (fsm->m_cur_state) {
        ui_sprite_fsm_state_update(fsm->m_cur_state, delta);
    }
}

void ui_sprite_fsm_ins_visit_actions(
    ui_sprite_fsm_ins_t fsm,
    void (*visitor)(void * ctx, ui_sprite_fsm_action_t action), void * ctx)
{
    ui_sprite_fsm_state_t state;

    TAILQ_FOREACH(state, &fsm->m_states, m_next_for_ins) {
        ui_sprite_fsm_action_t action;
        TAILQ_FOREACH(action, &state->m_actions, m_next_for_state) {
            visitor(ctx, action);
            if (strcmp(action->m_meta->m_name, "run-fsm") == 0) {
                ui_sprite_fsm_ins_visit_actions((ui_sprite_fsm_ins_t)ui_sprite_fsm_action_data(action), visitor, ctx);
            }
        }
    }
}

static void ui_sprite_fsm_ins_path_print(write_stream_t s, ui_sprite_fsm_ins_t fsm) {
    const char * state_name;

    if (fsm->m_parent) {
        ui_sprite_fsm_ins_path_print(s, fsm->m_parent);
        stream_putc(s, '.');
    }

    state_name = fsm->m_cur_state ? fsm->m_cur_state->m_name : "???";

    if (fsm->m_parent == NULL) {
        stream_printf(s, "Fsm[%s]", state_name);
    }
    else {
        ui_sprite_fsm_action_t action = ui_sprite_fsm_action_from_data(fsm);
        stream_printf(s, "%s[%s]", action->m_name, state_name);
    }
}

const char * ui_sprite_fsm_ins_path(ui_sprite_fsm_ins_t fsm) {
    ui_sprite_fsm_module_t module = fsm->m_module;
    return ui_sprite_fsm_dump_path(fsm, &module->m_dump_buffer);
}

const char * ui_sprite_fsm_dump_path(ui_sprite_fsm_ins_t fsm, mem_buffer_t buffer) {
    struct write_stream_buffer s = CPE_WRITE_STREAM_BUFFER_INITIALIZER(buffer);

    mem_buffer_clear_data(buffer);

    ui_sprite_fsm_ins_path_print((write_stream_t)&s, fsm);

    stream_putc((write_stream_t)&s, 0);

    return mem_buffer_make_continuous(buffer, 0);
}

