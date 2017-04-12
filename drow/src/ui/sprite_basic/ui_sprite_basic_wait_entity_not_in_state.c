#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "ui/sprite/ui_sprite_world.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite/ui_sprite_group.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_meta.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui/sprite_fsm/ui_sprite_fsm_component.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_calc.h"
#include "ui/sprite_cfg/ui_sprite_cfg_loader.h"
#include "ui_sprite_basic_wait_entity_not_in_state_i.h"
#include "protocol/ui/sprite_basic/ui_sprite_basic_data.h"
#include "protocol/ui/sprite_basic/ui_sprite_basic_evt.h"

ui_sprite_basic_wait_entity_not_in_state_t ui_sprite_basic_wait_entity_not_in_state_create(ui_sprite_fsm_state_t fsm_state, const char * name) {
	ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_create(fsm_state, name, UI_SPRITE_BASIC_WAIT_ENTITY_NOT_IN_STATE_NAME);
    return fsm_action ? ui_sprite_fsm_action_data(fsm_action) : NULL;
}

void ui_sprite_basic_wait_entity_not_in_state_free(ui_sprite_basic_wait_entity_not_in_state_t wait_entity_not_in_state) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_from_data(wait_entity_not_in_state);
    ui_sprite_fsm_action_free(fsm_action);
}

int ui_sprite_basic_wait_entity_not_in_state_set_entity(ui_sprite_basic_wait_entity_not_in_state_t wait_entity_not_in_state, const char * entity) {
	ui_sprite_basic_module_t module = wait_entity_not_in_state->m_module;

    if (wait_entity_not_in_state->m_cfg_entity) {
        if (wait_entity_not_in_state->m_cfg_entity) mem_free(module->m_alloc, wait_entity_not_in_state->m_cfg_entity);
    }
    
	if (entity) {
		wait_entity_not_in_state->m_cfg_entity = cpe_str_mem_dup_trim(module->m_alloc, entity);
		return wait_entity_not_in_state->m_cfg_entity == NULL ? -1 : 0;
	}
	else {
		wait_entity_not_in_state->m_cfg_entity = NULL;
		return 0;
	}
}

const char * ui_sprite_basic_wait_entity_not_in_state_entity(ui_sprite_basic_wait_entity_not_in_state_t wait_entity_not_in_state) {
	return wait_entity_not_in_state->m_cfg_entity;
}

int ui_sprite_basic_wait_entity_not_in_state_set_state(ui_sprite_basic_wait_entity_not_in_state_t wait_entity_not_in_state, const char * state) {
	ui_sprite_basic_module_t module = wait_entity_not_in_state->m_module;

    if (wait_entity_not_in_state->m_cfg_state) {
        if (wait_entity_not_in_state->m_cfg_state) mem_free(module->m_alloc, wait_entity_not_in_state->m_cfg_state);
    }
    
	if (state) {
		wait_entity_not_in_state->m_cfg_state = cpe_str_mem_dup_trim(module->m_alloc, state);
		return wait_entity_not_in_state->m_cfg_state == NULL ? -1 : 0;
	}
	else {
		wait_entity_not_in_state->m_cfg_state = NULL;
		return 0;
	}
}

const char * ui_sprite_basic_wait_entity_not_in_state_state(ui_sprite_basic_wait_entity_not_in_state_t wait_entity_not_in_state) {
	return wait_entity_not_in_state->m_cfg_state;
}

static int ui_sprite_basic_wait_entity_not_in_state_enter(ui_sprite_fsm_action_t fsm_action, void * ctx) {
	ui_sprite_basic_module_t module = ctx;
	ui_sprite_basic_wait_entity_not_in_state_t wait_entity_not_in_state = ui_sprite_fsm_action_data(fsm_action);
	ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);

    assert(wait_entity_not_in_state->m_entity == 0);
    assert(wait_entity_not_in_state->m_group == 0);
    assert(wait_entity_not_in_state->m_state == NULL);

    if (wait_entity_not_in_state->m_cfg_entity) {
        if (wait_entity_not_in_state->m_cfg_entity[0] == '*') {
            ui_sprite_group_t group;
            const char * str_group;

            str_group = ui_sprite_fsm_action_check_calc_str(&module->m_tmp_buff, wait_entity_not_in_state->m_cfg_entity + 1, fsm_action, NULL, module->m_em);
            if (str_group == NULL) {
                CPE_ERROR(
                    module->m_em, "entity %d(%s): %s: calc group name from %s fail!",
                    ui_sprite_entity_id(entity), ui_sprite_entity_name(entity),
                    UI_SPRITE_BASIC_WAIT_ENTITY_NOT_IN_STATE_NAME,
                    wait_entity_not_in_state->m_cfg_entity + 1);
                return -1;
            }

            group = ui_sprite_group_find_by_name(ui_sprite_entity_world(entity), str_group);
            if (group == NULL) {
                CPE_ERROR(
                    module->m_em, "entity %d(%s): %s: group %s not exist!",
                    ui_sprite_entity_id(entity), ui_sprite_entity_name(entity),
                    UI_SPRITE_BASIC_WAIT_ENTITY_NOT_IN_STATE_NAME, str_group);
                return -1;
            }
            wait_entity_not_in_state->m_group = ui_sprite_group_id(group);
        }
        else {
            if (ui_sprite_fsm_action_check_calc_entity_id(
                    &wait_entity_not_in_state->m_entity, wait_entity_not_in_state->m_cfg_entity, fsm_action, NULL, module->m_em)
                != 0)
            {
                CPE_ERROR(
                    module->m_em, "entity %d(%s): %s: calc entity id from %s fail!",
                    ui_sprite_entity_id(entity), ui_sprite_entity_name(entity),
                    UI_SPRITE_BASIC_WAIT_ENTITY_NOT_IN_STATE_NAME,
                    wait_entity_not_in_state->m_cfg_entity);
                return -1;
            }
        }
    }

    if (wait_entity_not_in_state->m_cfg_state == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): %s: state not configured!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity),
            UI_SPRITE_BASIC_WAIT_ENTITY_NOT_IN_STATE_NAME);
        return -1;
    }
    else {
        wait_entity_not_in_state->m_state = ui_sprite_fsm_action_check_calc_str_dup(module->m_alloc, wait_entity_not_in_state->m_cfg_state, fsm_action, NULL, module->m_em);
        if (wait_entity_not_in_state->m_state == NULL) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): %s: calc state from %s fail!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity),
                UI_SPRITE_BASIC_WAIT_ENTITY_NOT_IN_STATE_NAME,
                wait_entity_not_in_state->m_cfg_state);
            return -1;
        }
    }

    return ui_sprite_fsm_action_start_update(fsm_action);
}

static void ui_sprite_basic_wait_entity_not_in_state_exit(ui_sprite_fsm_action_t fsm_action, void * ctx) {
	ui_sprite_basic_module_t module = ctx;
	ui_sprite_basic_wait_entity_not_in_state_t wait_entity_not_in_state = ui_sprite_fsm_action_data(fsm_action);
	//ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);

    wait_entity_not_in_state->m_group = 0;
    wait_entity_not_in_state->m_entity = 0;

    if(wait_entity_not_in_state->m_state) {
        mem_free(module->m_alloc, wait_entity_not_in_state->m_state);
        wait_entity_not_in_state->m_state = NULL;
    }
}

static int ui_sprite_basic_wait_entity_not_in_state_init(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_basic_wait_entity_not_in_state_t wait_entity_not_in_state = ui_sprite_fsm_action_data(fsm_action);
    bzero(wait_entity_not_in_state, sizeof(*wait_entity_not_in_state));

	wait_entity_not_in_state->m_module = ctx;
	wait_entity_not_in_state->m_cfg_entity = NULL;
	wait_entity_not_in_state->m_cfg_state = NULL;
    wait_entity_not_in_state->m_group = 0;
	wait_entity_not_in_state->m_entity = 0;
	wait_entity_not_in_state->m_state = NULL;

    return 0;
}

static void ui_sprite_basic_wait_entity_not_in_state_clear(ui_sprite_fsm_action_t fsm_action, void * ctx) {
	ui_sprite_basic_module_t module = ctx;
	ui_sprite_basic_wait_entity_not_in_state_t wait_entity_not_in_state = ui_sprite_fsm_action_data(fsm_action);

    assert(wait_entity_not_in_state->m_entity == 0);
    assert(wait_entity_not_in_state->m_state == NULL);
    
	if (wait_entity_not_in_state->m_cfg_entity) {
		mem_free(module->m_alloc, wait_entity_not_in_state->m_cfg_entity);
		wait_entity_not_in_state->m_cfg_entity = NULL;
	}

	if (wait_entity_not_in_state->m_cfg_state) {
		mem_free(module->m_alloc, wait_entity_not_in_state->m_cfg_state);
		wait_entity_not_in_state->m_cfg_state = NULL;
	}
}

static int ui_sprite_basic_wait_entity_not_in_state_copy(ui_sprite_fsm_action_t to, ui_sprite_fsm_action_t from, void * ctx) {
	ui_sprite_basic_module_t module = ctx;
	ui_sprite_basic_wait_entity_not_in_state_t to_check_entity = ui_sprite_fsm_action_data(to);
	ui_sprite_basic_wait_entity_not_in_state_t from_check_entity = ui_sprite_fsm_action_data(from);

	if (ui_sprite_basic_wait_entity_not_in_state_init(to, ctx)) return -1;

	if (from_check_entity->m_cfg_entity) {
		to_check_entity->m_cfg_entity = cpe_str_mem_dup(module->m_alloc, from_check_entity->m_cfg_entity);
	}

	if (from_check_entity->m_cfg_state) {
		to_check_entity->m_cfg_state = cpe_str_mem_dup(module->m_alloc, from_check_entity->m_cfg_state);
	}
    
    return 0;
}

static void ui_sprite_basic_wait_entity_not_in_state_update(ui_sprite_fsm_action_t fsm_action, void * ctx, float delta) {
    ui_sprite_basic_module_t module = ctx;
	ui_sprite_basic_wait_entity_not_in_state_t wait_entity_not_in_state = ui_sprite_fsm_action_data(fsm_action);
	ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_entity_t check_entity = NULL;
    ui_sprite_fsm_component_fsm_t component_fsm;
    
    if (wait_entity_not_in_state->m_group) {
        ui_sprite_group_t group;
        group = ui_sprite_group_find_by_id(ui_sprite_entity_world(entity), wait_entity_not_in_state->m_group);
        if (group) {
            check_entity = ui_sprite_group_first_entity(group);
        }
    }
    else if (wait_entity_not_in_state->m_entity) {
        check_entity = ui_sprite_entity_find_by_id(ui_sprite_entity_world(entity), wait_entity_not_in_state->m_entity);
    }
    else {
        check_entity = entity;
    }

    if (check_entity == NULL) {
        ui_sprite_fsm_action_stop_update(fsm_action);
        return;
    }

    component_fsm = ui_sprite_fsm_component_find(check_entity);
    if (component_fsm == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): %s: check entity %d(%s) no fsm component!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity),
            UI_SPRITE_BASIC_WAIT_ENTITY_NOT_IN_STATE_NAME,
            ui_sprite_entity_id(check_entity), ui_sprite_entity_name(check_entity));
        ui_sprite_fsm_action_stop_update(fsm_action);
        return;
    }

    if (!ui_sprite_fsm_is_in_state((ui_sprite_fsm_ins_t)component_fsm, wait_entity_not_in_state->m_state)) {
        ui_sprite_fsm_action_stop_update(fsm_action);
        return;
    }
}

static ui_sprite_fsm_action_t ui_sprite_basic_wait_entity_not_in_state_load(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg) {
    ui_sprite_basic_module_t module = ctx;
    ui_sprite_basic_wait_entity_not_in_state_t check_entity = ui_sprite_basic_wait_entity_not_in_state_create(fsm_state, name);
	const char * str_value;

    if ((str_value  = cfg_get_string(cfg, "entity", NULL))) {
        if (ui_sprite_basic_wait_entity_not_in_state_set_entity(check_entity, str_value) != 0) {
            CPE_ERROR(
                module->m_em, "%s: create wait_entity_not_in_state action: set entity %s fail",
                ui_sprite_basic_module_name(module), str_value);
            ui_sprite_basic_wait_entity_not_in_state_free(check_entity);
            return NULL;
        }
    }

    if ((str_value  = cfg_get_string(cfg, "state", NULL))) {
        if (ui_sprite_basic_wait_entity_not_in_state_set_state(check_entity, str_value) != 0) {
            CPE_ERROR(
                module->m_em, "%s: load: set state %s fail",
                UI_SPRITE_BASIC_WAIT_ENTITY_NOT_IN_STATE_NAME, str_value);
            ui_sprite_basic_wait_entity_not_in_state_free(check_entity);
            return NULL;
        }
    }
    else {
        CPE_ERROR(
            module->m_em, "%s: load: state not configured",
            UI_SPRITE_BASIC_WAIT_ENTITY_NOT_IN_STATE_NAME);
        ui_sprite_basic_wait_entity_not_in_state_free(check_entity);
        return NULL;
    }
    
    return ui_sprite_fsm_action_from_data(check_entity);
}

int ui_sprite_basic_wait_entity_not_in_state_regist(ui_sprite_basic_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_create(
        module->m_fsm_module,
        UI_SPRITE_BASIC_WAIT_ENTITY_NOT_IN_STATE_NAME,
        sizeof(struct ui_sprite_basic_wait_entity_not_in_state));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: anim send event register: meta create fail",
            ui_sprite_basic_module_name(module));
        return -1;
    }

    ui_sprite_fsm_action_meta_set_enter_fun(meta, ui_sprite_basic_wait_entity_not_in_state_enter, module);
    ui_sprite_fsm_action_meta_set_exit_fun(meta, ui_sprite_basic_wait_entity_not_in_state_exit, module);
    ui_sprite_fsm_action_meta_set_init_fun(meta, ui_sprite_basic_wait_entity_not_in_state_init, module);
    ui_sprite_fsm_action_meta_set_copy_fun(meta, ui_sprite_basic_wait_entity_not_in_state_copy, module);
    ui_sprite_fsm_action_meta_set_free_fun(meta, ui_sprite_basic_wait_entity_not_in_state_clear, module);
    ui_sprite_fsm_action_meta_set_update_fun(meta, ui_sprite_basic_wait_entity_not_in_state_update, module);

    if (module->m_loader) {
        if (ui_sprite_cfg_loader_add_action_loader(module->m_loader, UI_SPRITE_BASIC_WAIT_ENTITY_NOT_IN_STATE_NAME, ui_sprite_basic_wait_entity_not_in_state_load, module) != 0) {
            ui_sprite_fsm_action_meta_free(meta);
            return -1;
        }
    }

    return 0;
}

void ui_sprite_basic_wait_entity_not_in_state_unregist(ui_sprite_basic_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_find(module->m_fsm_module, UI_SPRITE_BASIC_WAIT_ENTITY_NOT_IN_STATE_NAME);
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: send event unregister: meta not exist",
            ui_sprite_basic_module_name(module));
        return;
    }

    ui_sprite_fsm_action_meta_free(meta);

    if (module->m_loader) {
        ui_sprite_cfg_loader_remove_action_loader(module->m_loader, UI_SPRITE_BASIC_WAIT_ENTITY_NOT_IN_STATE_NAME);
    }
}

const char * UI_SPRITE_BASIC_WAIT_ENTITY_NOT_IN_STATE_NAME = "wait-entity-not-in-state";

