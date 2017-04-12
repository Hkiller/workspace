#include "cpe/pal/pal_strings.h"
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "render/utils/ui_vector_2.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_meta.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui/sprite_cfg/ui_sprite_cfg_loader.h"
#include "ui_sprite_2d_flip_i.h"
#include "ui_sprite_2d_module_i.h"
#include "ui_sprite_2d_transform_i.h"
#include "protocol/ui/sprite_2d/ui_sprite_2d_evt.h"

ui_sprite_2d_flip_t
ui_sprite_2d_flip_create(ui_sprite_fsm_state_t fsm_state, const char * name) {
    ui_sprite_fsm_action_t fsm_action;
    fsm_action = ui_sprite_fsm_action_create(fsm_state, name, UI_SPRITE_2D_FLIP_NAME);
    return fsm_action ? ui_sprite_fsm_action_data(fsm_action) : NULL;
}

void ui_sprite_2d_flip_free(ui_sprite_2d_flip_t show_anim) {
    ui_sprite_fsm_action_free(ui_sprite_fsm_action_from_data(show_anim));
}

static int ui_sprite_2d_flip_update_flip_to_entity(
    ui_sprite_2d_flip_t flip, 
    uint32_t flip_to_entity_id, const char * flip_to_entity_name, uint8_t pos_of_flip_entity,
    uint8_t process_x, uint8_t process_y)
{
    ui_sprite_2d_module_t module = flip->m_module;
	ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_from_data(flip);
	ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_2d_transform_t transform = ui_sprite_2d_transform_find(entity);
	ui_sprite_entity_t flip_to_entity;
    ui_sprite_2d_transform_t flip_to_transform;
    ui_vector_2 flip_to_pos;
    ui_vector_2 self_pos;
    uint8_t flip_x;
    uint8_t flip_y;

    if (flip_to_entity_id > 0) {
        flip_to_entity = ui_sprite_entity_find_by_id(ui_sprite_entity_world(entity), flip_to_entity_id);
        if (flip_to_entity == NULL) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): on flip to entity: entity %d not exist!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), flip_to_entity_id);
            return -1;
        }
    }
    else if (flip_to_entity_name[0]) {
        flip_to_entity = ui_sprite_entity_find_by_name(ui_sprite_entity_world(entity), flip_to_entity_name);
        if (flip_to_entity == NULL) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): on flip to entity: entity %s not exist!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), flip_to_entity_name);
            return -1;
        }
    }
    else {
        CPE_ERROR(
            module->m_em, "entity %d(%s): on flip to entity: entity not configured!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }

    flip_to_transform = ui_sprite_2d_transform_find(flip_to_entity);
    if (flip_to_transform == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): on flip to entity: entity %d(%s) no transform!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity),
            ui_sprite_entity_id(flip_to_entity), ui_sprite_entity_name(flip_to_entity));
        return -1;
    }

    flip_to_pos = ui_sprite_2d_transform_world_pos(flip_to_transform, pos_of_flip_entity, UI_SPRITE_2D_TRANSFORM_POS_ADJ_ALL);
    self_pos = ui_sprite_2d_transform_world_pos(transform, UI_SPRITE_2D_TRANSFORM_POS_ORIGIN, 0);

    flip_x = transform->m_data.transform.flip_x;
    flip_y = transform->m_data.transform.flip_y;

    if (process_x) {
        if (self_pos.x > flip_to_pos.x) {
            flip_x = 1;
        }
        else if (self_pos.x < flip_to_pos.x) {
            flip_x = 0;
        }
    }

    if (process_y) {
        if (self_pos.y > flip_to_pos.y) {
            flip_y = 1;
        }
        else if (self_pos.y < flip_to_pos.y) {
            flip_y = 0;
        }
    }

    ui_sprite_2d_transform_set_flip(transform, flip_x, flip_y);

    return 0;
}

static void ui_sprite_2d_flip_to_entity(void * ctx, ui_sprite_event_t evt) {
    ui_sprite_2d_flip_t flip = ctx;
    ui_sprite_2d_module_t module = flip->m_module;
	ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_from_data(flip);
	ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    uint8_t pos_of_flip_entity;

    UI_SPRITE_EVT_2D_FLIP_TO_ENTITY const * evt_data = evt->data;

    pos_of_flip_entity = ui_sprite_2d_transform_pos_policy_from_str(evt_data->pos_of_entity);
    if (pos_of_flip_entity == 0) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): on flip to entity: pos of entity %s is unknown!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), evt_data->pos_of_entity);
        ui_sprite_fsm_action_sync_update(fsm_action, 0);
        return;
    }

    ui_sprite_2d_flip_update_flip_to_entity(
        flip, evt_data->entity_id, evt_data->entity_name, pos_of_flip_entity,
        evt_data->process_x, evt_data->process_y);

	ui_sprite_fsm_action_sync_update(fsm_action, 0);
}

static void ui_sprite_2d_flip_follow_entity(void * ctx, ui_sprite_event_t evt) {
    ui_sprite_2d_flip_t flip = ctx;
    ui_sprite_2d_module_t module = flip->m_module;
	ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_from_data(flip);
	ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    uint8_t pos_of_flip_entity;

    UI_SPRITE_EVT_2D_FLIP_FOLLOW_ENTITY const * evt_data = evt->data;

    pos_of_flip_entity = ui_sprite_2d_transform_pos_policy_from_str(evt_data->pos_of_entity);
    if (pos_of_flip_entity == 0) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): on flip follow entity: pos of entity %s is unknown!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), evt_data->pos_of_entity);
        ui_sprite_fsm_action_sync_update(fsm_action, 0);
        return;
    }

    if (ui_sprite_2d_flip_update_flip_to_entity(
            flip, evt_data->entity_id, evt_data->entity_name, pos_of_flip_entity,
            evt_data->process_x, evt_data->process_y)
        != 0)
    {
        CPE_ERROR(
            module->m_em, "entity %d(%s): on flip follow entity: init flip fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        ui_sprite_fsm_action_sync_update(fsm_action, 0);
        return;
    }

    flip->m_flip_entity_id = evt_data->entity_id;
    cpe_str_dup(flip->m_flip_entity_name, sizeof(flip->m_flip_entity_name), evt_data->entity_name);
    flip->m_flip_to_entity_pos = pos_of_flip_entity;
    flip->m_process_x = evt_data->process_x;
    flip->m_process_y = evt_data->process_y;

	ui_sprite_fsm_action_sync_update(fsm_action, 1);
}

static int ui_sprite_2d_flip_enter(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_2d_flip_t flip = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_2d_module_t module = flip->m_module;

    if (ui_sprite_fsm_action_add_event_handler(
            fsm_action, ui_sprite_event_scope_self, 
            "ui_sprite_evt_2d_flip_to_entity", ui_sprite_2d_flip_to_entity, flip) != 0)
    {
        CPE_ERROR(module->m_em, "camera flip enter: add eventer handler fail!");
        return -1;
	}	

    if (ui_sprite_fsm_action_add_event_handler(
            fsm_action, ui_sprite_event_scope_self, 
            "ui_sprite_evt_2d_flip_follow_entity", ui_sprite_2d_flip_follow_entity, flip) != 0)
    {
        CPE_ERROR(module->m_em, "camera flip enter: add eventer handler fail!");
        return -1;
	}	

    return 0;
}

static void ui_sprite_2d_flip_update(ui_sprite_fsm_action_t fsm_action, void * ctx, float delta) {
    ui_sprite_2d_flip_t flip = ctx;
    ui_sprite_2d_module_t module = flip->m_module;
	ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);

    if (ui_sprite_2d_flip_update_flip_to_entity(
            flip, flip->m_flip_entity_id, flip->m_flip_entity_name, flip->m_flip_to_entity_pos,
            flip->m_process_x, flip->m_process_y)
        != 0)
    {
        CPE_ERROR(
            module->m_em, "entity %d(%s): update flip follow entity: flip!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        
        flip->m_flip_entity_id = 0;
        flip->m_flip_entity_name[0] = 0;

        ui_sprite_fsm_action_sync_update(fsm_action, 0);
        return;
    }
}

static void ui_sprite_2d_flip_exit(ui_sprite_fsm_action_t fsm_action, void * ctx) {
}

static int ui_sprite_2d_flip_init(ui_sprite_fsm_action_t fsm_action, void * ctx) {
	ui_sprite_2d_flip_t flip = ui_sprite_fsm_action_data(fsm_action);

	bzero(flip, sizeof(*flip));
	flip->m_module = ctx;
	return 0;
}

static int ui_sprite_2d_flip_copy(ui_sprite_fsm_action_t to, ui_sprite_fsm_action_t from, void * ctx) {
    ui_sprite_2d_flip_init(to, ctx);
    return 0;
}

static void ui_sprite_2d_flip_clear(ui_sprite_fsm_action_t fsm_action, void * ctx) {
}

static ui_sprite_fsm_action_t ui_sprite_2d_flip_load(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg) {
	ui_sprite_2d_module_t module = ctx;
	ui_sprite_2d_flip_t p2d_flip = ui_sprite_2d_flip_create(fsm_state, name);

	if (p2d_flip == NULL) {
		CPE_ERROR(module->m_em, "%s: create anim_2d_flip action: create fail!", ui_sprite_2d_module_name(module));
		return NULL;
	}

	return ui_sprite_fsm_action_from_data(p2d_flip);
}

int ui_sprite_2d_flip_regist(ui_sprite_2d_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_create(
        module->m_fsm_module, UI_SPRITE_2D_FLIP_NAME, sizeof(struct ui_sprite_2d_flip));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: %s register: meta create fail",
            ui_sprite_2d_module_name(module), UI_SPRITE_2D_FLIP_NAME);
        return -1;
    }

    ui_sprite_fsm_action_meta_set_init_fun(meta, ui_sprite_2d_flip_init, module);
    ui_sprite_fsm_action_meta_set_copy_fun(meta, ui_sprite_2d_flip_copy, module);
    ui_sprite_fsm_action_meta_set_enter_fun(meta, ui_sprite_2d_flip_enter, module);
    ui_sprite_fsm_action_meta_set_exit_fun(meta, ui_sprite_2d_flip_exit, module);
    ui_sprite_fsm_action_meta_set_free_fun(meta, ui_sprite_2d_flip_clear, module);
	ui_sprite_fsm_action_meta_set_update_fun(meta, ui_sprite_2d_flip_update, module);

    if (module->m_loader) {
        if (ui_sprite_cfg_loader_add_action_loader(module->m_loader, UI_SPRITE_2D_FLIP_NAME, ui_sprite_2d_flip_load, module) != 0) {
            ui_sprite_fsm_action_meta_free(meta);
            return -1;
        }
    }
    
    return 0;
}

void ui_sprite_2d_flip_unregist(ui_sprite_2d_module_t module) {
	ui_sprite_fsm_action_meta_t meta;

	meta = ui_sprite_fsm_action_meta_find(module->m_fsm_module, UI_SPRITE_2D_FLIP_NAME);
	if (meta == NULL) {
		CPE_ERROR(
			module->m_em, "%s: %s unregister: meta not exist",
			ui_sprite_2d_module_name(module), UI_SPRITE_2D_FLIP_NAME);
		return;
	}

	ui_sprite_fsm_action_meta_free(meta);

    if (module->m_loader) {
        ui_sprite_cfg_loader_remove_action_loader(module->m_loader, UI_SPRITE_2D_FLIP_NAME);
    }
}

const char * UI_SPRITE_2D_FLIP_NAME = "2d-flip";
