#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "ui/sprite/ui_sprite_world.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_meta.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_calc.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui/sprite_cfg/ui_sprite_cfg_loader.h"
#include "ui_sprite_basic_wait_event_i.h"

ui_sprite_basic_wait_event_t ui_sprite_basic_wait_event_create(ui_sprite_fsm_state_t fsm_state, const char * name) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_create(fsm_state, name, UI_SPRITE_BASIC_WAIT_EVENT_NAME);
    return fsm_action ? ui_sprite_fsm_action_data(fsm_action) : NULL;
}

void ui_sprite_basic_wait_event_free(ui_sprite_basic_wait_event_t wait_event) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_from_data(wait_event);
    ui_sprite_fsm_action_free(fsm_action);
}

int ui_sprite_basic_wait_event_set_event(ui_sprite_basic_wait_event_t wait_event, const char * event) {
    ui_sprite_basic_module_t module = wait_event->m_module;

    if (wait_event->m_event) mem_free(module->m_alloc, wait_event->m_event);

    if (event) {
        wait_event->m_event = cpe_str_mem_dup_trim(module->m_alloc, event);
        return wait_event->m_event == NULL ? -1 : 0;
    }
    else {
        wait_event->m_event = NULL;
        return 0;
    }
}

const char * ui_sprite_basic_wait_event_event(ui_sprite_basic_wait_event_t wait_event) {
    return wait_event->m_event;
}

int ui_sprite_basic_wait_event_set_condition(ui_sprite_basic_wait_event_t wait_event, const char * condition) {
    ui_sprite_basic_module_t module = wait_event->m_module;

    if (wait_event->m_condition) mem_free(module->m_alloc, wait_event->m_condition);

    if (condition) {
        wait_event->m_condition = cpe_str_mem_dup_trim(module->m_alloc, condition);
        return wait_event->m_condition == NULL ? -1 : 0;
    }
    else {
        wait_event->m_condition = NULL;
        return 0;
    }
}

const char * ui_sprite_basic_wait_event_condition(ui_sprite_basic_wait_event_t wait_event) {
    return wait_event->m_condition;
}

static void ui_sprite_basic_wait_on_event(void * ctx, ui_sprite_event_t evt) {
    ui_sprite_fsm_action_t fsm_action = ctx;
    ui_sprite_basic_wait_event_t wait_event = ui_sprite_fsm_action_data(fsm_action);

    if (wait_event->m_condition) {
        struct dr_data_source ds;
        uint8_t result;

        ds.m_data.m_meta = evt->meta;
        ds.m_data.m_data = (void*)evt->data;
        ds.m_data.m_size = evt->size;
        ds.m_next = NULL;

        if (ui_sprite_fsm_action_try_calc_bool(&result, wait_event->m_condition, fsm_action, &ds, wait_event->m_module->m_em) != 0) {
            ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
            CPE_ERROR(
                wait_event->m_module->m_em, "entity %d(%s): wait event: %s: calc condition %s fail!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), wait_event->m_event, wait_event->m_condition);
            return;
        }

        if (!result) return;
    }

    ui_sprite_fsm_action_stop_update(fsm_action);
}

static int ui_sprite_basic_wait_event_enter(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_basic_wait_event_t wait_event = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);

    if (wait_event->m_event == NULL) {
        CPE_ERROR(
            wait_event->m_module->m_em, "entity %d(%s): wait event: enter: no event!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }

    if (ui_sprite_fsm_action_add_event_handler(
            fsm_action, ui_sprite_event_scope_self,
            wait_event->m_event, ui_sprite_basic_wait_on_event, fsm_action) != 0)
    {
        CPE_ERROR(
            wait_event->m_module->m_em, "entity %d(%s): wait event: enter: create event handler fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }
    
    ui_sprite_fsm_action_start_update(fsm_action);

    return 0;
}

static void ui_sprite_basic_wait_event_update(ui_sprite_fsm_action_t fsm_action, void * ctx, float delta_s) {
}

static void ui_sprite_basic_wait_event_exit(ui_sprite_fsm_action_t fsm_action, void * ctx) {
}

static int ui_sprite_basic_wait_event_init(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_basic_wait_event_t wait_event = ui_sprite_fsm_action_data(fsm_action);
    wait_event->m_module = ctx;
	wait_event->m_event = NULL;
	wait_event->m_condition = NULL;
    return 0;
}

static void ui_sprite_basic_wait_event_clear(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_basic_module_t module = ctx;
    ui_sprite_basic_wait_event_t wait_event = ui_sprite_fsm_action_data(fsm_action);

    if (wait_event->m_event) {
        mem_free(module->m_alloc, wait_event->m_event);
        wait_event->m_event = NULL;
    }

    if (wait_event->m_condition) {
        mem_free(module->m_alloc, wait_event->m_condition);
        wait_event->m_condition = NULL;
    }
}

static int ui_sprite_basic_wait_event_copy(ui_sprite_fsm_action_t to, ui_sprite_fsm_action_t from, void * ctx) {
    ui_sprite_basic_module_t module = ctx;
    ui_sprite_basic_wait_event_t to_wait_event = ui_sprite_fsm_action_data(to);
    ui_sprite_basic_wait_event_t from_wait_event = ui_sprite_fsm_action_data(from);

    if (ui_sprite_basic_wait_event_init(to, ctx)) return -1;

    if (from_wait_event->m_event) {
        to_wait_event->m_event = cpe_str_mem_dup(module->m_alloc, from_wait_event->m_event);
    }

    if (from_wait_event->m_condition) {
        to_wait_event->m_condition = cpe_str_mem_dup(module->m_alloc, from_wait_event->m_condition);
    }

    return 0;
}

static ui_sprite_fsm_action_t ui_sprite_basic_wait_event_load(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg) {
    ui_sprite_basic_module_t module = ctx;
    ui_sprite_basic_wait_event_t wait_event = ui_sprite_basic_wait_event_create(fsm_state, name);

    if (wait_event == NULL) {
        CPE_ERROR(module->m_em, "%s: create wait_event action: create fail!", ui_sprite_basic_module_name(module));
        return NULL;
    }

    if (ui_sprite_basic_wait_event_set_event(wait_event, cfg_get_string(cfg, "event", NULL)) != 0) {
        CPE_ERROR(
            module->m_em, "%s: create wait_event action: set event fail",
            ui_sprite_basic_module_name(module));
        ui_sprite_basic_wait_event_free(wait_event);
        return NULL;
    }

    if (ui_sprite_basic_wait_event_set_condition(wait_event, cfg_get_string(cfg, "event-condition", NULL)) != 0) {
        CPE_ERROR(
            module->m_em, "%s: create wait_event action: set condition fail",
            ui_sprite_basic_module_name(module));
        ui_sprite_basic_wait_event_free(wait_event);
        return NULL;
    }

    return ui_sprite_fsm_action_from_data(wait_event);
}

int ui_sprite_basic_wait_event_regist(ui_sprite_basic_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_create(module->m_fsm_module, UI_SPRITE_BASIC_WAIT_EVENT_NAME, sizeof(struct ui_sprite_basic_wait_event));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: anim wait event register: meta create fail",
            ui_sprite_basic_module_name(module));
        return -1;
    }

    ui_sprite_fsm_action_meta_set_enter_fun(meta, ui_sprite_basic_wait_event_enter, module);
    ui_sprite_fsm_action_meta_set_update_fun(meta, ui_sprite_basic_wait_event_update, module);
    ui_sprite_fsm_action_meta_set_exit_fun(meta, ui_sprite_basic_wait_event_exit, module);
    ui_sprite_fsm_action_meta_set_init_fun(meta, ui_sprite_basic_wait_event_init, module);
    ui_sprite_fsm_action_meta_set_copy_fun(meta, ui_sprite_basic_wait_event_copy, module);
    ui_sprite_fsm_action_meta_set_free_fun(meta, ui_sprite_basic_wait_event_clear, module);

    if (module->m_loader) {
        if (ui_sprite_cfg_loader_add_action_loader(module->m_loader, UI_SPRITE_BASIC_WAIT_EVENT_NAME, ui_sprite_basic_wait_event_load, module) != 0) {
            ui_sprite_fsm_action_meta_free(meta);
            return -1;
        }
    }

    return 0;
}

void ui_sprite_basic_wait_event_unregist(ui_sprite_basic_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_find(module->m_fsm_module, UI_SPRITE_BASIC_WAIT_EVENT_NAME);
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: wait event unregister: meta not exist",
            ui_sprite_basic_module_name(module));
        return;
    }

    ui_sprite_fsm_action_meta_free(meta);

    if (module->m_loader) {
        ui_sprite_cfg_loader_remove_action_loader(module->m_loader, UI_SPRITE_BASIC_WAIT_EVENT_NAME);
    }
}

const char * UI_SPRITE_BASIC_WAIT_EVENT_NAME = "wait-event";

