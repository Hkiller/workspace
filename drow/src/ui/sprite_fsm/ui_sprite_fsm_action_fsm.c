#include <assert.h>
#include "cpe/utils/string_utils.h"
#include "cpe/dr/dr_data.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "gd/app/app_context.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite_fsm/ui_sprite_fsm_component.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_calc.h"
#include "ui_sprite_fsm_ins_i.h"
#include "ui_sprite_fsm_ins_state_i.h"
#include "ui_sprite_fsm_ins_action_i.h"
#include "ui_sprite_fsm_action_meta_i.h"
#include "ui_sprite_fsm_action_fsm_i.h"
#include "protocol/ui/sprite_fsm/ui_sprite_fsm_evt.h"

ui_sprite_fsm_action_fsm_t ui_sprite_fsm_action_fsm_create(ui_sprite_fsm_state_t fsm_state, const char * name) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_create(fsm_state, name, UI_SPRITE_FSM_ACTION_FSM_NAME);
    return fsm_action ? ui_sprite_fsm_action_data(fsm_action) : NULL;
}

dr_data_t ui_sprite_fsm_action_fsm_data(ui_sprite_fsm_action_fsm_t action_fsm) {
    return &action_fsm->m_data;
}

int ui_sprite_fsm_action_fsm_set_data(ui_sprite_fsm_action_fsm_t action_fsm, dr_data_t data) {
    if (action_fsm->m_data.m_data) {
        mem_free(action_fsm->m_ins.m_module->m_alloc, action_fsm->m_data.m_data);
    }

    if (data == NULL) {
        action_fsm->m_data.m_meta = NULL;
        action_fsm->m_data.m_data = NULL;
        action_fsm->m_data.m_size = 0;
    }
    else {
        action_fsm->m_data.m_meta = data->m_meta;
        action_fsm->m_data.m_size = data->m_size;
        action_fsm->m_data.m_data = mem_alloc(action_fsm->m_ins.m_module->m_alloc, data->m_size);
        if (action_fsm->m_data.m_data == NULL) {
            return -1;
        }
        memcpy(action_fsm->m_data.m_data, data->m_data, data->m_size);
    }

    return 0;
}

int ui_sprite_fsm_action_fsm_init_data(ui_sprite_fsm_action_fsm_t action_fsm, LPDRMETA data_meta, size_t data_capacity) {
    if (data_capacity == 0) data_capacity = dr_meta_size(data_meta);
    
    if (action_fsm->m_data.m_data) {
        mem_free(action_fsm->m_ins.m_module->m_alloc, action_fsm->m_data.m_data);
    }

    action_fsm->m_data.m_meta = data_meta;
    action_fsm->m_data.m_size = data_capacity;
    action_fsm->m_data.m_data = mem_calloc(action_fsm->m_ins.m_module->m_alloc, data_capacity);

    if (action_fsm->m_data.m_data == NULL) {
        action_fsm->m_data.m_meta = NULL;
        action_fsm->m_data.m_size = 0;
        return -1;
    }

    dr_meta_set_defaults(action_fsm->m_data.m_data, data_capacity, data_meta, DR_SET_DEFAULTS_POLICY_NO_DEFAULT_IGNORE);
    
    return 0;
}

const char * ui_sprite_fsm_action_fsm_load_from(ui_sprite_fsm_action_fsm_t action_fsm) {
    return action_fsm->m_cfg_load_from;
}

int ui_sprite_fsm_action_fsm_set_load_from(ui_sprite_fsm_action_fsm_t action_fsm, const char * load_from) {
    if (action_fsm->m_cfg_load_from) {
        mem_free(action_fsm->m_ins.m_module->m_alloc, action_fsm->m_cfg_load_from);
    }

    if (load_from) {
        action_fsm->m_cfg_load_from = cpe_str_mem_dup_trim(action_fsm->m_ins.m_module->m_alloc, load_from);
        if (action_fsm->m_cfg_load_from == NULL) return -1;
    }
    else {
        action_fsm->m_cfg_load_from = NULL;
    }

    return 0;
}

static int ui_sprite_fsm_action_fsm_enter(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    struct ui_sprite_fsm_action_fsm * action_fsm = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_fsm_ins_t fsm = &action_fsm->m_ins;
    ui_sprite_fsm_module_t module = fsm->m_module;
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);

    if (action_fsm->m_cfg_load_from) {
        ui_sprite_fsm_ins_t proto_fsm;
        const char * load_from;

        load_from = ui_sprite_fsm_action_check_calc_str(gd_app_tmp_buffer(module->m_app), action_fsm->m_cfg_load_from, fsm_action, NULL, module->m_em);
        if (load_from == NULL) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): %s: action %s(%s) calc load-from from %s fail",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), ui_sprite_fsm_ins_path(fsm),
                fsm_action->m_meta->m_name, fsm_action->m_name, action_fsm->m_cfg_load_from);
            return -1;
        }
        
        proto_fsm = ui_sprite_fsm_proto_find(ui_sprite_entity_world(entity), load_from);
        if (proto_fsm == NULL) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): %s: action %s(%s) load fsm from %s: proto entity no fsm component",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), ui_sprite_fsm_ins_path(fsm),
                fsm_action->m_meta->m_name, fsm_action->m_name, load_from);
            return -1;
        }
        load_from = NULL;
        
        ui_sprite_fsm_ins_reinit(fsm);
        if (ui_sprite_fsm_ins_copy(fsm, proto_fsm) != 0) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): %s: action %s(%s) load fsm from %s: copy fail!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), ui_sprite_fsm_ins_path(fsm),
                fsm_action->m_meta->m_name, fsm_action->m_name, action_fsm->m_cfg_load_from);
            ui_sprite_fsm_ins_reinit(fsm);
            return -1;
        }
    }

    if (TAILQ_EMPTY(&fsm->m_states)) return 0;

    if (ui_sprite_fsm_ins_enter(fsm) != 0) {
        if (action_fsm->m_cfg_load_from) {
            ui_sprite_fsm_ins_reinit(fsm);
        }
        return -1;
    }

    ui_sprite_fsm_ins_check(fsm);

    if (fsm->m_cur_state == NULL) {
        if (ui_sprite_entity_debug(entity)) {
            CPE_INFO(
                module->m_em, "entity %d(%s): %s: action %s(%s) all actions done",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), ui_sprite_fsm_ins_path(fsm),
                fsm_action->m_meta->m_name, fsm_action->m_name);
        }
        return 0;
    }

    if (ui_sprite_fsm_action_start_update(fsm_action) != 0) return -1;

    return 0;
}

static void ui_sprite_fsm_action_fsm_exit(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    struct ui_sprite_fsm_action_fsm * action_fsm = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_fsm_ins_t fsm = &action_fsm->m_ins;

    ui_sprite_fsm_ins_exit(fsm);

    if (action_fsm->m_cfg_load_from) {
        ui_sprite_fsm_ins_reinit(fsm);
    }
}

static void ui_sprite_fsm_action_fsm_update(ui_sprite_fsm_action_t fsm_action, void * ctx, float delta) {
    struct ui_sprite_fsm_action_fsm * action_fsm = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_fsm_module_t module = ctx;
    ui_sprite_fsm_ins_t fsm = &action_fsm->m_ins;

    assert(fsm_action->m_runing_state == ui_sprite_fsm_action_state_runing);

    ui_sprite_fsm_ins_update(fsm, delta);

    if (fsm->m_cur_state == NULL) {
        ui_sprite_fsm_action_stop_update(fsm_action);

        if (action_fsm->m_cfg_load_from) {
            ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
            if (ui_sprite_entity_debug(entity)) {
                CPE_INFO(
                    module->m_em, "entity %d(%s): %s: action %s(%s): auto clear!",
                    ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), ui_sprite_fsm_ins_path(fsm),
                    fsm_action->m_meta->m_name, fsm_action->m_name);
            }

            ui_sprite_fsm_ins_reinit(&action_fsm->m_ins);
        }
    }
}

static int ui_sprite_fsm_action_fsm_init(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    struct ui_sprite_fsm_action_fsm * action_fsm = ui_sprite_fsm_action_data(fsm_action);

    assert(fsm_action->m_state);

    ui_sprite_fsm_ins_init(&action_fsm->m_ins, ctx, fsm_action->m_state->m_ins);
    action_fsm->m_cfg_load_from = NULL;
    action_fsm->m_data.m_meta = NULL;
    action_fsm->m_data.m_data = NULL;
    action_fsm->m_data.m_size = 0;

    return 0;
}

static void ui_sprite_fsm_action_fsm_fini(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    struct ui_sprite_fsm_action_fsm * action_fsm = ui_sprite_fsm_action_data(fsm_action);

    ui_sprite_fsm_ins_fini(&action_fsm->m_ins);
    ui_sprite_fsm_action_fsm_set_data(action_fsm, NULL);
    ui_sprite_fsm_action_fsm_set_load_from(action_fsm, NULL);
}

static int ui_sprite_fsm_action_fsm_copy(ui_sprite_fsm_action_t to, ui_sprite_fsm_action_t from, void * ctx) {
    ui_sprite_fsm_module_t module = ctx;
    struct ui_sprite_fsm_action_fsm * to_action_fsm = ui_sprite_fsm_action_data(to);
    struct ui_sprite_fsm_action_fsm * from_action_fsm = ui_sprite_fsm_action_data(from);

    ui_sprite_fsm_action_fsm_init(to, ctx);

    if (ui_sprite_fsm_ins_copy(&to_action_fsm->m_ins, &from_action_fsm->m_ins) != 0) {
        ui_sprite_fsm_action_fsm_fini(to, ctx);
        return -1;
    }

    if (from_action_fsm->m_data.m_meta) {
        if (ui_sprite_fsm_action_fsm_set_data(to_action_fsm, &from_action_fsm->m_data) != 0) {
            ui_sprite_fsm_action_fsm_fini(to, ctx);
            return -1;
        }
    }

    if (from_action_fsm->m_cfg_load_from) {
        to_action_fsm->m_cfg_load_from = cpe_str_mem_dup(module->m_alloc, from_action_fsm->m_cfg_load_from);
        if (to_action_fsm->m_cfg_load_from == NULL) {
            ui_sprite_fsm_action_fsm_fini(to, ctx);
            return -1;
        }
    }
    
    return 0;
}

int ui_sprite_fsm_action_fsm_regist(ui_sprite_fsm_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_create(module, UI_SPRITE_FSM_ACTION_FSM_NAME, sizeof(struct ui_sprite_fsm_action_fsm));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: fsm fsm_action register: meta create fail",
            ui_sprite_fsm_module_name(module));
        return -1;
    }

    ui_sprite_fsm_action_meta_set_enter_fun(meta, ui_sprite_fsm_action_fsm_enter, module);
    ui_sprite_fsm_action_meta_set_exit_fun(meta, ui_sprite_fsm_action_fsm_exit, module);
    ui_sprite_fsm_action_meta_set_init_fun(meta, ui_sprite_fsm_action_fsm_init, module);
    ui_sprite_fsm_action_meta_set_copy_fun(meta, ui_sprite_fsm_action_fsm_copy, module);
    ui_sprite_fsm_action_meta_set_free_fun(meta, ui_sprite_fsm_action_fsm_fini, module);
    ui_sprite_fsm_action_meta_set_update_fun(meta, ui_sprite_fsm_action_fsm_update, module);

    return 0;
}

void ui_sprite_fsm_action_fsm_unregist(ui_sprite_fsm_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_find(module, UI_SPRITE_FSM_ACTION_FSM_NAME);
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: fsm fsm_action unregister: meta not exist",
            ui_sprite_fsm_module_name(module));
        return;
    }

    ui_sprite_fsm_action_meta_free(meta);
}

const char * UI_SPRITE_FSM_ACTION_FSM_NAME = "run-fsm";
