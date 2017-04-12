#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "ui/sprite/ui_sprite_world.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_meta.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui/sprite_cfg/ui_sprite_cfg_loader.h"
#include "ui_sprite_basic_send_event_i.h"

ui_sprite_basic_send_event_t ui_sprite_basic_send_event_create(ui_sprite_fsm_state_t fsm_state, const char * name) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_create(fsm_state, name, UI_SPRITE_BASIC_SEND_EVENT_NAME);
    return fsm_action ? ui_sprite_fsm_action_data(fsm_action) : NULL;
}

void ui_sprite_basic_send_event_free(ui_sprite_basic_send_event_t send_event) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_from_data(send_event);
    ui_sprite_fsm_action_free(fsm_action);
}

int ui_sprite_basic_send_event_set_on_enter(ui_sprite_basic_send_event_t send_event, const char * on_enter) {
    ui_sprite_basic_module_t module = send_event->m_module;

    if (send_event->m_on_enter) mem_free(module->m_alloc, send_event->m_on_enter);

    if (on_enter) {
        send_event->m_on_enter = cpe_str_mem_dup(module->m_alloc, on_enter);
        return send_event->m_on_enter == NULL ? -1 : 0;
    }
    else {
        send_event->m_on_enter = NULL;
        return 0;
    }
}

const char * ui_sprite_basic_send_event_on_enter(ui_sprite_basic_send_event_t send_event) {
    return send_event->m_on_enter;
}

int ui_sprite_basic_send_event_set_on_exit(ui_sprite_basic_send_event_t send_event, const char * on_exit) {
    ui_sprite_basic_module_t module = send_event->m_module;

    if (send_event->m_on_exit) mem_free(module->m_alloc, send_event->m_on_exit);

    if (on_exit) {
        send_event->m_on_exit = cpe_str_mem_dup(module->m_alloc, on_exit);
        return send_event->m_on_exit == NULL ? -1 : 0;
    }
    else {
        send_event->m_on_exit = NULL;
        return 0;
    }
}

const char * ui_sprite_basic_send_event_on_exit(ui_sprite_basic_send_event_t send_event) {
    return send_event->m_on_exit;
}

static int ui_sprite_basic_send_event_enter(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_basic_send_event_t send_event = ui_sprite_fsm_action_data(fsm_action);

    if (send_event->m_on_enter) {
        ui_sprite_fsm_action_build_and_send_event(fsm_action, send_event->m_on_enter, NULL);
    }

    return 0;
}

static void ui_sprite_basic_send_event_exit(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_basic_send_event_t send_event = ui_sprite_fsm_action_data(fsm_action);

    if (send_event->m_on_exit) {
        ui_sprite_fsm_action_build_and_send_event(fsm_action, send_event->m_on_exit, NULL);
    }

}

static int ui_sprite_basic_send_event_init(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_basic_send_event_t send_event = ui_sprite_fsm_action_data(fsm_action);
    send_event->m_module = ctx;
	send_event->m_on_enter = NULL;
	send_event->m_on_exit = NULL;
    return 0;
}

static void ui_sprite_basic_send_event_clear(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_basic_module_t module = ctx;
    ui_sprite_basic_send_event_t send_event = ui_sprite_fsm_action_data(fsm_action);

    if (send_event->m_on_enter) {
        mem_free(module->m_alloc, send_event->m_on_enter);
        send_event->m_on_enter = NULL;
    }

    if (send_event->m_on_exit) {
        mem_free(module->m_alloc, send_event->m_on_exit);
        send_event->m_on_exit = NULL;
    }
}

static int ui_sprite_basic_send_event_copy(ui_sprite_fsm_action_t to, ui_sprite_fsm_action_t from, void * ctx) {
    ui_sprite_basic_module_t module = ctx;
    ui_sprite_basic_send_event_t to_send_event = ui_sprite_fsm_action_data(to);
    ui_sprite_basic_send_event_t from_send_event = ui_sprite_fsm_action_data(from);

    if (ui_sprite_basic_send_event_init(to, ctx)) return -1;

    if (from_send_event->m_on_enter) {
        to_send_event->m_on_enter = cpe_str_mem_dup(module->m_alloc, from_send_event->m_on_enter);
    }

    if (from_send_event->m_on_exit) {
        to_send_event->m_on_exit = cpe_str_mem_dup(module->m_alloc, from_send_event->m_on_exit);
    }

    return 0;
}

static ui_sprite_fsm_action_t ui_sprite_basic_send_event_load(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg) {
    ui_sprite_basic_module_t module = ctx;
    ui_sprite_basic_send_event_t send_event = ui_sprite_basic_send_event_create(fsm_state, name);

    if (send_event == NULL) {
        CPE_ERROR(module->m_em, "%s: create send_event action: create fail!", ui_sprite_basic_module_name(module));
        return NULL;
    }

    if (ui_sprite_basic_send_event_set_on_enter(send_event, cfg_get_string(cfg, "on-enter", NULL)) != 0) {
        CPE_ERROR(
            module->m_em, "%s: create send_event action: set on-enter fail",
            ui_sprite_basic_module_name(module));
        ui_sprite_basic_send_event_free(send_event);
        return NULL;
    }

    if (ui_sprite_basic_send_event_set_on_exit(send_event, cfg_get_string(cfg, "on-exit", NULL)) != 0) {
        CPE_ERROR(
            module->m_em, "%s: create send_event action: set on-exit fail",
            ui_sprite_basic_module_name(module));
        ui_sprite_basic_send_event_free(send_event);
        return NULL;
    }

    return ui_sprite_fsm_action_from_data(send_event);
}

int ui_sprite_basic_send_event_regist(ui_sprite_basic_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_create(module->m_fsm_module, UI_SPRITE_BASIC_SEND_EVENT_NAME, sizeof(struct ui_sprite_basic_send_event));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: anim send event register: meta create fail",
            ui_sprite_basic_module_name(module));
        return -1;
    }

    ui_sprite_fsm_action_meta_set_enter_fun(meta, ui_sprite_basic_send_event_enter, module);
    ui_sprite_fsm_action_meta_set_exit_fun(meta, ui_sprite_basic_send_event_exit, module);
    ui_sprite_fsm_action_meta_set_init_fun(meta, ui_sprite_basic_send_event_init, module);
    ui_sprite_fsm_action_meta_set_copy_fun(meta, ui_sprite_basic_send_event_copy, module);
    ui_sprite_fsm_action_meta_set_free_fun(meta, ui_sprite_basic_send_event_clear, module);

    if (module->m_loader) {
        if (ui_sprite_cfg_loader_add_action_loader(module->m_loader, UI_SPRITE_BASIC_SEND_EVENT_NAME, ui_sprite_basic_send_event_load, module) != 0) {
            ui_sprite_fsm_action_meta_free(meta);
            return -1;
        }
    }

    return 0;
}

void ui_sprite_basic_send_event_unregist(ui_sprite_basic_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_find(module->m_fsm_module, UI_SPRITE_BASIC_SEND_EVENT_NAME);
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: send event unregister: meta not exist",
            ui_sprite_basic_module_name(module));
        return;
    }

    ui_sprite_fsm_action_meta_free(meta);

    if (module->m_loader) {
        ui_sprite_cfg_loader_remove_action_loader(module->m_loader, UI_SPRITE_BASIC_SEND_EVENT_NAME);
    }
}

const char * UI_SPRITE_BASIC_SEND_EVENT_NAME = "send-event";

