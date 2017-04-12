#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "ui/sprite/ui_sprite_world.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite/ui_sprite_entity_attr.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_meta.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_calc.h"
#include "ui/sprite_cfg/ui_sprite_cfg_loader.h"
#include "ui_sprite_basic_wait_entity_destory_i.h"
#include "protocol/ui/sprite_basic/ui_sprite_basic_data.h"
#include "protocol/ui/sprite_basic/ui_sprite_basic_evt.h"

ui_sprite_basic_wait_entity_destory_t ui_sprite_basic_wait_entity_destory_create(ui_sprite_fsm_state_t fsm_state, const char * name) {
	ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_create(fsm_state, name, UI_SPRITE_BASIC_WAIT_ENTITY_DESTORY_NAME);
    return fsm_action ? ui_sprite_fsm_action_data(fsm_action) : NULL;
}

void ui_sprite_basic_wait_entity_destory_free(ui_sprite_basic_wait_entity_destory_t wait_entity_destory) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_from_data(wait_entity_destory);
    ui_sprite_fsm_action_free(fsm_action);
}

int ui_sprite_basic_wait_entity_destory_set_entity_id(ui_sprite_basic_wait_entity_destory_t wait_entity_destory, const char * entity_id) {
	ui_sprite_basic_module_t module = wait_entity_destory->m_module;

    if (wait_entity_destory->m_cfg_entity_id) {
        if (wait_entity_destory->m_cfg_entity_id) mem_free(module->m_alloc, wait_entity_destory->m_cfg_entity_id);
    }
    
	if (entity_id) {
		wait_entity_destory->m_cfg_entity_id = cpe_str_mem_dup(module->m_alloc, entity_id);
		return wait_entity_destory->m_cfg_entity_id == NULL ? -1 : 0;
	}
	else {
		wait_entity_destory->m_cfg_entity_id = NULL;
		return 0;
	}
}

const char * ui_sprite_basic_wait_entity_destory_entity_id(ui_sprite_basic_wait_entity_destory_t wait_entity_destory) {
	return wait_entity_destory->m_cfg_entity_id;
}

static int ui_sprite_basic_wait_entity_destory_enter(ui_sprite_fsm_action_t fsm_action, void * ctx) {
	ui_sprite_basic_module_t module = ctx;
	ui_sprite_basic_wait_entity_destory_t wait_entity_destory = ui_sprite_fsm_action_data(fsm_action);
	ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);

    if (wait_entity_destory->m_cfg_entity_id == NULL) {
		CPE_ERROR(
			module->m_em, "entity %d(%s): wait_entity_destory: entity-id not configured!",
			ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }

    if (ui_sprite_fsm_action_check_calc_entity_id(
            &wait_entity_destory->m_entity_id, wait_entity_destory->m_cfg_entity_id, fsm_action, NULL, module->m_em)
        != 0)
    {
		CPE_ERROR(
			module->m_em, "entity %d(%s): wait_entity_destory: calc entity id from %s fail!",
			ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), wait_entity_destory->m_cfg_entity_id);
		return -1;
	}

    if (ui_sprite_entity_find_by_id(ui_sprite_fsm_action_to_world(fsm_action), wait_entity_destory->m_entity_id) != NULL) {
		ui_sprite_fsm_action_start_update(fsm_action);
    }
    
    return 0;
}

static void ui_sprite_basic_wait_entity_destory_exit(ui_sprite_fsm_action_t fsm_action, void * ctx) {
	ui_sprite_basic_module_t module = ctx;
	ui_sprite_basic_wait_entity_destory_t wait_entity_destory = ui_sprite_fsm_action_data(fsm_action);
	ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);

	if (wait_entity_destory->m_cfg_entity_id == NULL) {
		CPE_ERROR(
			module->m_em, "entity %d(%s): exit group: group name not set!",
			ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
		return;
	}

	if (ui_sprite_entity_debug(entity)) {
		CPE_INFO(
			module->m_em, "entity %d(%s): exit action check entity exist",
			ui_sprite_entity_id(entity), ui_sprite_entity_name(entity)
			);
	}
}

static int ui_sprite_basic_wait_entity_destory_init(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_basic_wait_entity_destory_t wait_entity_destory = ui_sprite_fsm_action_data(fsm_action);
    bzero(wait_entity_destory, sizeof(*wait_entity_destory));

	wait_entity_destory->m_module = ctx;
	wait_entity_destory->m_entity_id = 0;
	wait_entity_destory->m_cfg_entity_id = NULL;

    return 0;
}

static void ui_sprite_basic_wait_entity_destory_clear(ui_sprite_fsm_action_t fsm_action, void * ctx) {
	ui_sprite_basic_module_t module = ctx;
	ui_sprite_basic_wait_entity_destory_t wait_entity_destory = ui_sprite_fsm_action_data(fsm_action);

	if (wait_entity_destory->m_cfg_entity_id) {
		mem_free(module->m_alloc, wait_entity_destory->m_cfg_entity_id);
		wait_entity_destory->m_cfg_entity_id = NULL;
	}
}

static int ui_sprite_basic_wait_entity_destory_copy(ui_sprite_fsm_action_t to, ui_sprite_fsm_action_t from, void * ctx) {
	ui_sprite_basic_module_t module = ctx;
	ui_sprite_basic_wait_entity_destory_t to_check_entity = ui_sprite_fsm_action_data(to);
	ui_sprite_basic_wait_entity_destory_t from_check_entity = ui_sprite_fsm_action_data(from);

	if (ui_sprite_basic_wait_entity_destory_init(to, ctx)) return -1;

	if (from_check_entity->m_cfg_entity_id) {
		to_check_entity->m_cfg_entity_id = cpe_str_mem_dup(module->m_alloc, from_check_entity->m_cfg_entity_id);
	}

    return 0;
}

static void ui_sprite_basic_wait_entity_destory_update(ui_sprite_fsm_action_t fsm_action, void * ctx, float delta) {
	ui_sprite_basic_wait_entity_destory_t wait_entity_destory = ui_sprite_fsm_action_data(fsm_action);

    if (ui_sprite_entity_find_by_id(ui_sprite_fsm_action_to_world(fsm_action), wait_entity_destory->m_entity_id) == NULL) {
		ui_sprite_fsm_action_stop_update(fsm_action);
    }
}

static ui_sprite_fsm_action_t ui_sprite_basic_wait_entity_destory_load(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg) {
    ui_sprite_basic_module_t module = ctx;
    ui_sprite_basic_wait_entity_destory_t check_entity = ui_sprite_basic_wait_entity_destory_create(fsm_state, name);
	const char * entity_id;

    entity_id  = cfg_get_string(cfg, "entity-id", NULL);
    if (entity_id == NULL) {
        CPE_ERROR(
            module->m_em, "%s: create wait entity destory action: entity-id  not configured",
            ui_sprite_basic_module_name(module));
        return NULL;
    }


	if (ui_sprite_basic_wait_entity_destory_set_entity_id(check_entity, entity_id) != 0) {
		CPE_ERROR(
			module->m_em, "%s: create wait_entity_destory action: set entity_id %s fail",
			ui_sprite_basic_module_name(module), entity_id);
		ui_sprite_basic_wait_entity_destory_free(check_entity);
		return NULL;
	}

    return ui_sprite_fsm_action_from_data(check_entity);
}

int ui_sprite_basic_wait_entity_destory_regist(ui_sprite_basic_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_create(
        module->m_fsm_module,
        UI_SPRITE_BASIC_WAIT_ENTITY_DESTORY_NAME,
        sizeof(struct ui_sprite_basic_wait_entity_destory));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: anim send event register: meta create fail",
            ui_sprite_basic_module_name(module));
        return -1;
    }

    ui_sprite_fsm_action_meta_set_enter_fun(meta, ui_sprite_basic_wait_entity_destory_enter, module);
    ui_sprite_fsm_action_meta_set_exit_fun(meta, ui_sprite_basic_wait_entity_destory_exit, module);
    ui_sprite_fsm_action_meta_set_init_fun(meta, ui_sprite_basic_wait_entity_destory_init, module);
    ui_sprite_fsm_action_meta_set_copy_fun(meta, ui_sprite_basic_wait_entity_destory_copy, module);
    ui_sprite_fsm_action_meta_set_free_fun(meta, ui_sprite_basic_wait_entity_destory_clear, module);
    ui_sprite_fsm_action_meta_set_update_fun(meta, ui_sprite_basic_wait_entity_destory_update, module);

    if (module->m_loader) {
        if (ui_sprite_cfg_loader_add_action_loader(module->m_loader, UI_SPRITE_BASIC_WAIT_ENTITY_DESTORY_NAME, ui_sprite_basic_wait_entity_destory_load, module) != 0) {
            ui_sprite_fsm_action_meta_free(meta);
            return -1;
        }
    }

    return 0;
}

void ui_sprite_basic_wait_entity_destory_unregist(ui_sprite_basic_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_find(module->m_fsm_module, UI_SPRITE_BASIC_WAIT_ENTITY_DESTORY_NAME);
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: send event unregister: meta not exist",
            ui_sprite_basic_module_name(module));
        return;
    }

    ui_sprite_fsm_action_meta_free(meta);

    if (module->m_loader) {
        ui_sprite_cfg_loader_remove_action_loader(module->m_loader, UI_SPRITE_BASIC_WAIT_ENTITY_DESTORY_NAME);
    }
}

const char * UI_SPRITE_BASIC_WAIT_ENTITY_DESTORY_NAME = "wait-entity-destory";

